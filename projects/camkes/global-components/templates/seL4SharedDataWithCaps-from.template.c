/*#
 *#Copyright 2017, Data61
 *#Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 *#ABN 41 687 119 230.
 *#
 *#This software may be distributed and modified according to the terms of
 *#the BSD 2-Clause license. Note that NO WARRANTY is provided.
 *#See "LICENSE_BSD2.txt" for details.
 *#
 *#@TAG(DATA61_BSD)
  #*/
#include <camkes/dataport.h>
#include <stdint.h>
#include <stdlib.h>
#include <utils/util.h>

/*? macros.show_includes(me.instance.type.includes) ?*/

/*- set index = me.parent.from_ends.index(me) -*/

#define SHM_ALIGN (1 << 12)

/*- set dataport_symbol_name = "from_%d_%s_data" % (index, me.interface.name) -*/

struct {
    char content[ROUND_UP_UNSAFE(/*? macros.dataport_size(me.interface.type) ?*/,
        PAGE_SIZE_4K)];
} /*? dataport_symbol_name ?*/
        __attribute__((aligned(SHM_ALIGN)))
        __attribute__((section("shared_from_/*? index ?*/_/*? me.interface.name ?*/")));
/*- set perm = configuration[me.instance.name].get('%s_access' % me.interface.name) -*/
/*- if perm is not none and not re.match('^R?W?X?$', perm) -*/
  /*? raise(TemplateError('invalid permissions attribute %s.%s_access' % (me.instance.name, me.interface.name), configuration)) ?*/
/*- endif -*/
/*- do register_shared_variable('%s_data' % me.parent.name, dataport_symbol_name, perm if perm is not none else 'RWX') -*/
/*- do keep_symbol(dataport_symbol_name) -*/

volatile /*? macros.dataport_type(me.interface.type) ?*/ * /*? me.interface.name ?*/ =
    (volatile /*? macros.dataport_type(me.interface.type) ?*/ *) & from_/*? index ?*/_/*? me.interface.name ?*/_data;

/*- set id = composition.connections.index(me.parent) -*/

int /*? me.interface.name ?*/_wrap_ptr(dataport_ptr_t *p, void *ptr) {
    if ((uintptr_t)ptr < (uintptr_t)/*? me.interface.name ?*/ ||
            (uintptr_t)ptr >= (uintptr_t)/*? me.interface.name ?*/ + /*? macros.dataport_size(me.interface.type) ?*/) {
        return -1;
    }
    p->id = /*? id ?*/;
    p->offset = (off_t)((uintptr_t)ptr - (uintptr_t)/*? me.interface.name ?*/);
    return 0;
}

void * /*? me.interface.name ?*/_unwrap_ptr(dataport_ptr_t *p) {
    if (p->id == /*? id ?*/) {
        return (void*)((uintptr_t)/*? me.interface.name ?*/ + (uintptr_t)p->offset);
    } else {
        return NULL;
    }
}
