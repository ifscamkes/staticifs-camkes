/*
 * Copyright 2017, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */

/*# C fragment that represents the base of the buffer used for storing IPC messages #*/
/*? assert(isinstance(base, six.string_types)) ?*/
/*? assert(isinstance(buffer_size, six.string_types)) ?*/
/*# Whether 'base' is a separate memory region instead of the thread's IPC buffer #*/
/*? assert(isinstance(userspace_ipc, bool)) ?*/
/*# Whether or not we trust our partner #*/
/*? assert(isinstance(trust_partner, bool)) ?*/

#include <autoconf.h>
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <camkes/error.h>
#include <camkes/tls.h>
#include <sel4/sel4.h>
#include <camkes/dataport.h>
#include <utils/util.h>

/*? macros.show_includes(me.instance.type.includes) ?*/
/*? macros.show_includes(me.interface.type.includes) ?*/

/*- set BUFFER_BASE = c_symbol('BUFFER_BASE') -*/
#define /*? BUFFER_BASE ?*/ /*? base ?*/

/*- set methods_len = len(me.interface.type.methods) -*/
/*- set instance = me.instance.name -*/
/*- set interface = me.interface.name -*/
/*- set threads = list(six.moves.range(1, 2 + len(me.instance.type.provides) + len(me.instance.type.uses) + len(me.instance.type.emits) + len(me.instance.type.consumes))) -*/

/* Interface-specific error handling */
/*- set error_handler = '%s_error_handler' % me.interface.name -*/
/*- include 'error-handler.c' -*/

/*- for m in me.interface.type.methods -*/
    extern
    /*- if m.return_type is not none -*/
        /*- if m.return_type == 'string' -*/
            char *
        /*- else -*/
            /*? macros.show_type(m.return_type) ?*/
        /*- endif -*/
    /*- else -*/
        void
    /*- endif -*/
    /*? me.interface.name ?*/_/*? m.name ?*/(
      /*- for p in m.parameters -*/
        /*- if p.direction == 'in' -*/
          /*- if p.array -*/
            size_t /*? p.name ?*/_sz,
            /*- if p.type == 'string' -*/
              char **
            /*- else -*/
              const /*? macros.show_type(p.type) ?*/ *
            /*- endif -*/
          /*- elif p.type == 'string' -*/
            const char *
          /*- else -*/
            /*? macros.show_type(p.type) ?*/
          /*- endif -*/
          /*? p.name ?*/
        /*- else -*/
          /*? assert(p.direction in ['refin', 'out', 'inout']) ?*/
          /*- if p.array -*/
            /*- if p.direction == 'refin' -*/
              const
            /*- endif -*/
            size_t * /*? p.name ?*/_sz,
            /*- if p.type == 'string' -*/
              char ***
            /*- else -*/
              /*? macros.show_type(p.type) ?*/ **
            /*- endif -*/
          /*- elif p.type == 'string' -*/
            char **
          /*- else -*/
            /*- if p.direction == 'refin' -*/
              const
            /*- endif -*/
            /*? macros.show_type(p.type) ?*/ *
          /*- endif -*/
          /*? p.name ?*/
        /*- endif -*/
        /*- if not loop.last -*/
          ,
        /*- endif -*/
      /*- endfor -*/
      /*- if len(m.parameters) == 0 -*/
        void
      /*- endif -*/
    );

/*- set name = m.name -*/
/*- set function = '%s_unmarshal_inputs' % m.name -*/
/*- set buffer = base -*/
/*- set size = buffer_size -*/
/*- set input_parameters = list(filter(lambda('x: x.direction in [\'refin\', \'in\', \'inout\']'), m.parameters)) -*/
/*- set allow_trailing_data = userspace_ipc -*/
/*- include 'unmarshal-inputs.c' -*/

/*- set function = '%s_marshal_outputs' % m.name -*/
/*- set output_parameters = list(filter(lambda('x: x.direction in [\'out\', \'inout\']'), m.parameters)) -*/
/*- set return_type = m.return_type -*/
/*- include 'marshal-outputs.c' -*/

/*- if m.return_type is not none -*/
  /*- if m.return_type == 'string' -*/
    /*- set array = False -*/
    /*- set name = '%s_ret_to' % m.name -*/
    /*- set type = 'char*' -*/
    /*- include 'thread_local.c' -*/
  /*- else -*/
    /*- set array = False -*/
    /*- set name = '%s_ret_to' % m.name -*/
    /*- set type = macros.show_type(m.return_type) -*/
    /*- include 'thread_local.c' -*/
  /*- endif -*/
/*- endif -*/
/*- for p in m.parameters -*/
  /*- if p.array -*/
    /*- set array = False -*/
    /*- set name = '%s_%s_sz_to' % (m.name, p.name) -*/
    /*- set type = 'size_t' -*/
    /*- include 'thread_local.c' -*/
    /*- if p.type == 'string' -*/
      /*- set array = False -*/
      /*- set name = '%s_%s_to' % (m.name, p.name) -*/
      /*- set type = 'char**' -*/
      /*- include 'thread_local.c' -*/
    /*- else -*/
      /*- set array = False -*/
      /*- set name = '%s_%s_to' % (m.name, p.name) -*/
      /*- set type = '%s*' % macros.show_type(p.type) -*/
      /*- include 'thread_local.c' -*/
    /*- endif -*/
  /*- elif p.type == 'string' -*/
    /*- set array = False -*/
    /*- set name = '%s_%s_to' % (m.name, p.name) -*/
    /*- set type = 'char*' -*/
    /*- include 'thread_local.c' -*/
  /*- else -*/
    /*- set array = False -*/
    /*- set name = '%s_%s_to' % (m.name, p.name) -*/
    /*- set type = macros.show_type(p.type) -*/
    /*- include 'thread_local.c' -*/
  /*- endif -*/
/*- endfor -*/

/*- endfor -*/

/*- set ep_obj = alloc_obj('ep', seL4_EndpointObject) -*/
/*- set ep = alloc_cap('ep', ep_obj, read=True, write=True) -*/

static seL4_Word /*? me.interface.name ?*/_badge = 0;

seL4_Word /*? me.interface.name ?*/_get_sender_id(void) {
    return /*? me.interface.name ?*/_badge;
}

/*- set call_tls_var = c_symbol('call_tls_var_to') -*/
/*- set array = False -*/
/*- set name = call_tls_var -*/
/*- if methods_len <= 1 -*/
  /*- set type = 'unsigned int' -*/
  /*- include 'thread_local.c' -*/
/*- elif methods_len <= 2 ** 8 -*/
  /*- set type = 'uint8_t' -*/
  /*- include 'thread_local.c' -*/
/*- elif methods_len <= 2 ** 16 -*/
  /*- set type = 'uint16_t' -*/
  /*- include 'thread_local.c' -*/
/*- elif methods_len <= 2 ** 32 -*/
  /*- set type = 'uint32_t' -*/
  /*- include 'thread_local.c' -*/
/*- elif methods_len <= 2 ** 64 -*/
  /*- set type = 'uint64_t' -*/
  /*- include 'thread_local.c' -*/
/*- else -*/
  /*? raise(TemplateError('too many methods in interface %s' % me.interface.name)) ?*/
/*- endif -*/

/*- include 'array-typedef-check.c' -*/

/*- set p = Perspective(instance=me.instance.name, interface=me.interface.name) -*/
/*- set passive = options.realtime and configuration[me.instance.name].get(p['passive_attribute'], False) -*/

/*# Passive interface "run" functions must be passed a ntfn cap as part of the passive thread init protocol.
 *# As such if this is a passive interface, a different function prototype is needed for "run".
 #*/
int
/*- if passive -*/
    /*- set init_ntfn = c_symbol() -*/
    /*? me.interface.name ?*/__run_passive(seL4_CPtr /*? init_ntfn ?*/)
/*- else -*/
    /*? me.interface.name ?*/__run(void)
/*- endif -*/
{

    /*# Check any typedefs we have been given are not arrays. #*/
    /*- include 'call-array-typedef-check.c' -*/

    /*- if options.realtime -*/
            /*- set reply_cap_slot = alloc('reply_cap_slot', seL4_RTReplyObject) -*/
    /*- else -*/
        /*- if me.might_block() -*/
            /* We're going to need a CNode cap in order to save our pending reply
             * caps in the future.
             */
            /*- set cnode = alloc_cap('cnode', my_cnode, write=True) -*/
            /*- set reply_cap_slot = alloc_cap('reply_cap_slot', None) -*/
            camkes_get_tls()->cnode_cap = /*? cnode ?*/;
        /*- endif -*/
    /*- endif -*/

    /*- set info = c_symbol('info') -*/
    /*- if passive -*/
        /* This interface has a passive thread, must let the control thread know before waiting */
        seL4_MessageInfo_t /*? info ?*/ = /*? generate_seL4_SignalRecv(options,
                                                                       init_ntfn,
                                                                       info, ep,
                                                                       '&%s_badge' % me.interface.name,
                                                                       reply_cap_slot) ?*/;
    /*- else -*/
       /* This interface has an active thread, just wait for an RPC */
        seL4_MessageInfo_t /*? info ?*/ = /*? generate_seL4_Recv(options, ep,
                                                                 '&%s_badge' % me.interface.name,
                                                                 reply_cap_slot) ?*/;
    /*- endif -*/

    while (1) {

        /*- set buffer = c_symbol('buffer') -*/
        void * /*? buffer ?*/ UNUSED = (void*)/*? BUFFER_BASE ?*/;

        /*- set size = c_symbol('size') -*/
        unsigned /*? size ?*/ UNUSED =
        /*- if userspace_ipc -*/
            /*? buffer_size ?*/;
        /*- else -*/
            seL4_MessageInfo_get_length(/*? info ?*/) * sizeof(seL4_Word);
            assert(/*? size ?*/ <= seL4_MsgMaxLength * sizeof(seL4_Word));
        /*- endif -*/

        /*- set call = c_symbol('call') -*/
        /*- set call_ptr = c_symbol('call_ptr') -*/
        /*- if methods_len <= 1 -*/
          unsigned /*? call ?*/ UNUSED;
          unsigned * /*? call_ptr ?*/ = TLS_PTR(/*? call_tls_var ?*/, /*? call ?*/);
          * /*? call_ptr ?*/ = 0;
        /*- elif methods_len <= 2 ** 8 -*/
          uint8_t /*? call ?*/ UNUSED;
          uint8_t * /*? call_ptr ?*/ = TLS_PTR(/*? call_tls_var ?*/, /*? call ?*/);
        /*- elif methods_len <= 2 ** 16 -*/
          uint16_t /*? call ?*/ UNUSED;
          uint16_t * /*? call_ptr ?*/ = TLS_PTR(/*? call_tls_var ?*/, /*? call ?*/);
        /*- elif methods_len <= 2 ** 32 -*/
          uint32_t /*? call ?*/ UNUSED;
          uint32_t * /*? call_ptr ?*/ = TLS_PTR(/*? call_tls_var ?*/, /*? call ?*/);
        /*- elif methods_len <= 2 ** 64 -*/
          uint64_t /*? call ?*/ UNUSED;
          uint64_t * /*? call_ptr ?*/ = TLS_PTR(/*? call_tls_var ?*/, /*? call ?*/);
        /*- else -*/
          /*? raise(TemplateError('too many methods in interface %s' % me.interface.name)) ?*/
        /*- endif -*/
        /*- if methods_len > 1 -*/
            ERR_IF(sizeof(* /*? call_ptr ?*/) > /*? size ?*/, /*? error_handler ?*/, ((camkes_error_t){
                    .type = CE_MALFORMED_RPC_PAYLOAD,
                    .instance = "/*? instance ?*/",
                    .interface = "/*? interface ?*/",
                    .description = "truncated message encountered while unmarshalling method index in /*? me.interface.name ?*/",
                    .length = /*? size ?*/,
                    .current_index = sizeof(* /*? call_ptr ?*/),
                }), ({
                    /*? info ?*/ = /*? generate_seL4_Recv(options, ep,
                                                          '&%s_badge' % me.interface.name,
                                                          reply_cap_slot) ?*/;
                    continue;
                }));

            memcpy(/*? call_ptr ?*/, /*? buffer ?*/, sizeof(* /*? call_ptr ?*/));
        /*- endif -*/

        switch (* /*? call_ptr ?*/) {
            /*- for i, m in enumerate(me.interface.type.methods) -*/
                case /*? i ?*/: { /*? '%s%s%s%s%s' % ('/', '* ', m.name, ' *', '/') ?*/
                    /*# Declare parameters. #*/
                    /*- for p in m.parameters -*/

                        /*- if p.array -*/
                            size_t /*? p.name ?*/_sz UNUSED;
                            size_t * /*? p.name ?*/_sz_ptr = TLS_PTR(/*? m.name ?*/_/*? p.name ?*/_sz_to, /*? p.name ?*/_sz);
                            /*- if p.type == 'string' -*/
                                char ** /*? p.name ?*/ UNUSED = NULL;
                                char *** /*? p.name ?*/_ptr = TLS_PTR(/*? m.name ?*/_/*? p.name ?*/_to, /*? p.name ?*/);
                            /*- else -*/
                                /*? macros.show_type(p.type) ?*/ * /*? p.name ?*/ UNUSED = NULL;
                                /*? macros.show_type(p.type) ?*/ ** /*? p.name ?*/_ptr = TLS_PTR(/*? m.name ?*/_/*? p.name ?*/_to, /*? p.name ?*/);
                            /*- endif -*/
                        /*- elif p.type == 'string' -*/
                            char * /*? p.name ?*/ UNUSED = NULL;
                            char ** /*? p.name ?*/_ptr = TLS_PTR(/*? m.name ?*/_/*? p.name ?*/_to, /*? p.name ?*/);
                        /*- else -*/
                            /*? macros.show_type(p.type) ?*/ /*? p.name ?*/ UNUSED;
                            /*? macros.show_type(p.type) ?*/ * /*? p.name ?*/_ptr = TLS_PTR(/*? m.name ?*/_/*? p.name ?*/_to, /*? p.name ?*/);
                        /*- endif -*/
                    /*- endfor -*/

                    /* Unmarshal parameters */
                    /*- set function = '%s_unmarshal_inputs' % m.name -*/
                    /*- set input_parameters = list(filter(lambda('x: x.direction in [\'refin\', \'in\', \'inout\']'), m.parameters)) -*/
                    /*- set err = c_symbol('error') -*/
                    int /*? err ?*/ = /*- include 'call-unmarshal-inputs.c' -*/;
                    if (unlikely(/*? err ?*/ != 0)) {
                        /* Error in unmarshalling; return to event loop. */
                        /*? info ?*/ = /*? generate_seL4_Recv(options, ep,
                                                              '&%s_badge' % me.interface.name,
                                                              reply_cap_slot) ?*/;
                        continue;
                    }

                    /*- if not options.realtime and me.might_block() -*/
                        /* We need to save the reply cap because the user's implementation may
                         * perform operations that overwrite or discard it.
                         */
                        /*- set result = c_symbol() -*/
                        /*? assert(reply_cap_slot is defined and reply_cap_slot > 0) ?*/
                        int /*? result ?*/ UNUSED = camkes_declare_reply_cap(/*? reply_cap_slot ?*/);
                        ERR_IF(/*? result ?*/ != 0, /*? error_handler ?*/, ((camkes_error_t){
                                .type = CE_ALLOCATION_FAILURE,
                                .instance = "/*? instance ?*/",
                                .interface = "/*? interface ?*/",
                                .description = "failed to declare reply cap in /*? m.name ?*/",
                                .alloc_bytes = sizeof(seL4_CPtr),
                            }), ({
                                /*? info ?*/ = /*? generate_seL4_Recv(options, ep,
                                                                      '&%s_badge' % me.interface.name,
                                                                      reply_cap_slot) ?*/;
                                continue;
                            }));
                    /*- endif -*/

                    /* Call the implementation */
                    /*- set ret = c_symbol('ret') -*/
                    /*- set ret_sz = c_symbol('ret_sz') -*/
                    /*- set ret_ptr = c_symbol('ret_ptr') -*/
                    /*- set ret_sz_ptr = c_symbol('ret_sz_ptr') -*/
                    /*- if m.return_type is not none -*/
                        /*- if m.return_type == 'string' -*/
                            char * /*? ret ?*/ UNUSED;
                            char ** /*? ret_ptr ?*/ = TLS_PTR(/*? m.name ?*/_ret_to, /*? ret ?*/);
                        /*- else -*/
                            /*? macros.show_type(m.return_type) ?*/ /*? ret ?*/ UNUSED;
                            /*? macros.show_type(m.return_type) ?*/ * /*? ret_ptr ?*/ = TLS_PTR(/*? m.name ?*/_ret_to, /*? ret ?*/);
                        /*- endif -*/
                        * /*? ret_ptr ?*/ =
                    /*- endif -*/
                    /*? me.interface.name ?*/_/*? m.name ?*/(
                        /*- for p in m.parameters -*/
                            /*- if p.array -*/
                                /*- if p.direction == 'in' -*/
                                    *
                                /*- endif -*/
                                /*? p.name ?*/_sz_ptr,
                            /*- endif -*/
                            /*- if p.direction =='in' -*/
                                *
                            /*- endif -*/
                            /*? p.name ?*/_ptr
                            /*- if not loop.last -*/,/*- endif -*/
                        /*- endfor -*/
                    );

                    /*- set tls = c_symbol() -*/
                    camkes_tls_t * /*? tls ?*/ UNUSED = camkes_get_tls();

                    /* Marshal the response */
                    /*- set function = '%s_marshal_outputs' % m.name -*/
                    /*- set output_parameters = list(filter(lambda('x: x.direction in [\'out\', \'inout\']'), m.parameters)) -*/
                    /*- set return_type = m.return_type -*/
                    /*- set length = c_symbol('length') -*/
                    unsigned /*? length ?*/ = /*- include 'call-marshal-outputs.c' -*/;

                    /*# We no longer need anything we previously malloced #*/
                    /*- if m.return_type == 'string' -*/
                      free(* /*? ret_ptr ?*/);
                    /*- endif -*/
                    /*- for p in m.parameters -*/
                      /*- if p.array -*/
                        /*- if p.type == 'string' -*/
                          /*- set mcount = c_symbol() -*/
                          for (int /*? mcount ?*/ = 0; /*? mcount ?*/ < * /*? p.name ?*/_sz_ptr; /*? mcount ?*/ ++) {
                            free((* /*? p.name ?*/_ptr)[/*? mcount ?*/]);
                          }
                        /*- endif -*/
                        free(* /*? p.name ?*/_ptr);
                      /*- elif p.type == 'string' -*/
                        free(* /*? p.name ?*/_ptr);
                      /*- endif -*/
                    /*- endfor -*/

                    /* Check if there was an error during marshalling. We do
                     * this after freeing internal parameter variables to avoid
                     * leaking memory on errors.
                     */
                    if (unlikely(/*? length ?*/ == UINT_MAX)) {
                        /* Error occurred; return to event loop. */
                        /*? info ?*/ = /*? generate_seL4_Recv(options, ep,
                                                              '&%s_badge' % me.interface.name,
                                                              reply_cap_slot) ?*/;
                        continue;
                    }

                    /*? info ?*/ = seL4_MessageInfo_new(0, 0, 0, /* length */
                        /*- if userspace_ipc -*/
                            0
                        /*- else -*/
                            ROUND_UP_UNSAFE(/*? length ?*/, sizeof(seL4_Word)) / sizeof(seL4_Word)
                        /*- endif -*/
                    );

                    /* Send the response */
                    /*- if not options.realtime and me.might_block() -*/
                        assert(/*? tls ?*/ != NULL);
                        if (/*? tls ?*/->reply_cap_in_tcb) {
                            /*? tls ?*/->reply_cap_in_tcb = false;
                            /*? info ?*/ = /*? generate_seL4_ReplyRecv(options, ep,
                                                                       info,
                                                                       '&%s_badge' % me.interface.name,
                                                                       reply_cap_slot) ?*/;
                        } else {
                            /*- set error = c_symbol() -*/
                            seL4_Error /*? error ?*/ UNUSED = camkes_unprotect_reply_cap();
                            ERR_IF(/*? error ?*/ != seL4_NoError, /*? error_handler ?*/, ((camkes_error_t){
                                    .type = CE_SYSCALL_FAILED,
                                    .instance = "/*? instance ?*/",
                                    .interface = "/*? interface ?*/",
                                    .description = "failed to save reply cap in /*? m.name ?*/",
                                    .syscall = CNodeSaveCaller,
                                    .error = /*? error ?*/,
                                }), ({
                                    /*? info ?*/ = /*? generate_seL4_Recv(options,
                                                                          ep,
                                                                          '&%s_badge' % me.interface.name,
                                                                          reply_cap_slot) ?*/;
                                    continue;
                                }));

                            seL4_Send(/*? reply_cap_slot ?*/, /*? info ?*/);
                            /*? info ?*/ = /*? generate_seL4_Recv(options, ep,
                                                                  '&%s_badge' % me.interface.name,
                                                                  reply_cap_slot) ?*/;
                        }
                    /*- elif options.realtime -*/
                        /*? info ?*/ = /*? generate_seL4_ReplyRecv(options, ep,
                                                                   info,
                                                                   '&%s_badge' % me.interface.name,
                                                                   reply_cap_slot) ?*/;
                    /*- else -*/

                        /*- if not options.realtime and len(me.parent.from_ends) == 1 and len(me.parent.to_ends) == 1 and options.fspecialise_syscall_stubs and methods_len == 1 and m.return_type is none and len(m.parameters) == 0 -*/
#ifdef CONFIG_ARCH_ARM
#ifndef __SWINUM
    #define __SWINUM(x) ((x) & 0x00ffffff)
#endif
                            /* We don't need to send or receive any information, so
                             * we can call ReplyRecv with a custom syscall stub
                             * that reduces the overhead of the call. To explain
                             * where this deviates from the standard ReplyRecv
                             * stub:
                             *  - No asm clobbers because we're not receiving any
                             *    arguments in the message;
                             *  - The MessageInfo as an input only because we know
                             *    the return (a new Call) will be 0 as well; and
                             *  - Setup r7 and r1 first because they are preserved
                             *    across the syscall and this helps the compiler
                             *    make a tighter loop if necessary.
                             */
                            /*- set scno = c_symbol() -*/
                            register seL4_Word /*? scno ?*/ asm("r7") = seL4_SysReplyRecv;
                            /*- set info2 = c_symbol() -*/
                            register seL4_MessageInfo_t /*? info2 ?*/ asm("r1") = seL4_MessageInfo_new(0, 0, 0, 0);
                            /*- set src = c_symbol() -*/
                            register seL4_Word /*? src ?*/ asm("r0") = /*? ep ?*/;
                            asm volatile("swi %[swinum]"
                                /*- if trust_partner -*/
                                    :"+r"(/*? src ?*/)
                                    :[swinum]"i"(__SWINUM(seL4_SysReplyRecv)), "r"(/*? scno ?*/), "r"(/*? info2 ?*/)
                                /*- else -*/
                                    :"+r"(/*? src ?*/), "+r"(/*? info2 ?*/)
                                    :[swinum]"i"(__SWINUM(seL4_SysReplyRecv)), "r"(/*? scno ?*/)
                                    :"r2", "r3", "r4", "r5", "memory"
                                /*- endif -*/
                            );
                            /*? info ?*/ = /*? info2 ?*/; /*# Most probably, not necessary. #*/
                            break;
#endif
                        /*- endif -*/
                        /*? info ?*/ = /*? generate_seL4_ReplyRecv(options, ep,
                                                                   info,
                                                                   '&%s_badge' % me.interface.name,
                                                                   reply_cap_slot) ?*/;
                    /*- endif -*/

                    break;
                }
            /*- endfor -*/
            default: {
                ERR(/*? error_handler ?*/, ((camkes_error_t){
                        .type = CE_INVALID_METHOD_INDEX,
                        .instance = "/*? instance ?*/",
                        .interface = "/*? interface ?*/",
                        .description = "invalid method index received in /*? me.interface.name ?*/",
                        .lower_bound = 0,
                        .upper_bound = /*? methods_len ?*/ - 1,
                        .invalid_index = * /*? call_ptr ?*/,
                    }), ({
                        /*? info ?*/ = /*? generate_seL4_Recv(options, ep,
                                                              '&%s_badge' % me.interface.name,
                                                              reply_cap_slot) ?*/;
                        continue;
                    }));
            }
        }
    }

    UNREACHABLE();
}
