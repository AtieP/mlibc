#pragma once

#include <stdint.h>
#include <mlibc/tcb.hpp>
#include <bits/ensure.h>

namespace mlibc {

inline Tcb *get_current_tcb() {
	// On RISC-V, the TCB is below the thread pointer.
	uintptr_t tp = (uintptr_t)__builtin_thread_pointer();
	auto tcb = reinterpret_cast<Tcb *>(tp - sizeof(Tcb));
	__ensure(tcb == tcb->selfPointer);
	return tcb;
}

} // namespace mlibc
