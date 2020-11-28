#ifndef VTABLE_H
#define VTABLE_H


// Virtual table struct declaration
#define VTABLE_STRUCT                             struct Vtable_

// Virtual table field declaration
#define VTABLE_FIELD                              const Vtable_ *vtable_;

// Set instance's virtual type on pointer
#define VSETTYPE(INSTANCE, TYPENAME)              (INSTANCE)->vtable_ = &((INSTANCE)->VTYPE(TYPENAME));

// Set instance's virtual type
#define VSETTYPEI(INSTANCE, TYPENAME)             (INSTANCE).vtable_ = &((*INSTANCE).VTYPE(TYPENAME));

// Virtual subclass name
#define VTYPE(TYPENAME)                           vtable##TYPENAME

// Virtual subclass field of physical class
#define VTYPE_FIELD(TYPENAME)                     static const Vtable_ VTYPE(TYPENAME);

// Definition for the above static field
#define VTYPE_DEF(TYPENAME, CLSNAME)              const CLSNAME::Vtable_ CLSNAME::VTYPE(TYPENAME)

// Virtual table method declaration
#define VDECL(CLSNAME, RTYPE, FUNCNAME, ...)      RTYPE (CLSNAME::*FUNCNAME)(##__VA_ARGS__);

// Virtual method implementation name
#define VMIN(TYPENAME, METHODNAME)                virt_##TYPENAME##_##METHODNAME##_

// Virtual table call on instance
#define VCALLI(INSTANCE, FUNCNAME, ...)           ((INSTANCE).*((INSTANCE).vtable_->FUNCNAME))(__VA_ARGS__)

// Virtual table call on pointer
#define VCALL(INSTANCE, FUNCNAME, ...)            ((INSTANCE)->*((INSTANCE)->vtable_->FUNCNAME))(__VA_ARGS__)

// Virtual isinstance check
#define VISINST(INSTANCE, TYPENAME)               (&((INSTANCE)->VTYPE(TYPENAME)) == (INSTANCE)->vtable_)

// Virtual isinstance check
#define VISINSTI(INSTANCE, TYPENAME)              (&(INSTANCE).VTYPE(TYPENAME)) == (INSTANCE).vtable_)


#endif
