/*
Copyright (c) 2016 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#include "util/hash.h"
#include "kernel/expr_maps.h"
#include "kernel/instantiate.h"
#include "library/abstract_expr.h"
#include "library/cache_helper.h"
#include "library/fun_info.h"

namespace lean {
struct abstract_expr_cache {
    environment        m_env;
    expr_map<unsigned> m_hash_cache;
    expr_map<unsigned> m_weight_cache;
    abstract_expr_cache(environment const & env):m_env(env) {}
    environment const & env() const { return m_env; }
};

/* The abstract_expr_cache does not depend on the transparency mode */
typedef transparencyless_cache_compatibility_helper<abstract_expr_cache>
abstract_expr_cache_helper;

MK_THREAD_LOCAL_GET_DEF(abstract_expr_cache_helper, get_aech);

abstract_expr_cache & get_abstract_cache_for(type_context const & ctx) {
    return get_aech().get_cache_for(ctx);
}

#define EASY_HASH(e) {                                  \
    switch (e.kind()) {                                 \
    case expr_kind::Constant: case expr_kind::Local:    \
    case expr_kind::Meta:     case expr_kind::Sort:     \
    case expr_kind::Var:                                \
        return e.hash();                                \
    default:                                            \
        break;                                          \
    }                                                   \
}

struct abstract_hash_fn {
    type_context &                   m_ctx;
    expr_map<unsigned> &             m_cache;
    buffer<expr>                     m_locals;
    type_context::transparency_scope m_scope;

    abstract_hash_fn(type_context & ctx):
        m_ctx(ctx),
        m_cache(get_abstract_cache_for(ctx).m_hash_cache),
        m_scope(m_ctx, transparency_mode::All) {}

    expr instantiate_locals(expr const & e) {
        return instantiate_rev(e, m_locals.size(), m_locals.data());
    }

    expr push_local(name const & pp_name, expr const & type) {
        expr l = m_ctx.push_local(pp_name, instantiate_locals(type));
        m_locals.push_back(l);
        return l;
    }

    expr push_let(name const & pp_name, expr const & type, expr const & value) {
        expr l = m_ctx.push_let(pp_name, instantiate_locals(type), instantiate_locals(value));
        m_locals.push_back(l);
        return l;
    }

    void pop() {
        m_locals.pop_back();
    }

    unsigned hash(expr const & e) {
        EASY_HASH(e);

        auto it = m_cache.find(e);
        if (it != m_cache.end())
            return it->second;

        unsigned r = 0;

        switch (e.kind()) {
        case expr_kind::Constant: case expr_kind::Local:
        case expr_kind::Meta:     case expr_kind::Sort:
        case expr_kind::Var:
            lean_unreachable();
        case expr_kind::Lambda:
        case expr_kind::Pi:
            r = hash(binding_domain(e));
            push_local(binding_name(e), binding_domain(e));
            r = ::lean::hash(r, hash(binding_body(e)));
            pop();
            break;
        case expr_kind::Let:
            r = ::lean::hash(hash(let_type(e)), hash(let_value(e)));
            push_let(let_name(e), let_type(e), let_value(e));
            r = ::lean::hash(r, hash(let_body(e)));
            pop();
            break;
        case expr_kind::Macro:
            r = lean::hash(macro_num_args(e), [&](unsigned i) { return hash(macro_arg(e, i)); },
                           macro_def(e).hash());
            break;
        case expr_kind::App:
            buffer<expr> args;
            expr const & f = get_app_args(e, args);
            r = hash(f);
            fun_info info  = get_fun_info(m_ctx, instantiate_locals(f), args.size());
            unsigned i = 0;
            for (param_info const & pinfo : info.get_params_info()) {
                lean_assert(i < args.size());
                if (!pinfo.is_inst_implicit() && !pinfo.is_prop()) {
                    r = ::lean::hash(r, hash(args[i]));
                }
                i++;
            }
            /* Remark: the property (i == args.size()) does not necessarily hold here.
               This can happen whenever the arity of f depends on its arguments. */
            break;
        }
        m_cache.insert(mk_pair(e, r));
        return r;
    }

    unsigned operator()(expr const & e) {
        return hash(e);
    }
};

unsigned abstract_hash(type_context & ctx, expr const & e) {
    EASY_HASH(e);
    return abstract_hash_fn(ctx)(e);
}
}
