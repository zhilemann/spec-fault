# `spec-fault` - a syscall-free memory fault oracle

![a funny demo GIF](./.md/demo.gif)

## what is this?

`spec-fault` **"predicts" whether accessing said memory will trigger a fault** via [side-channel speculative execution][spectre-meltdown].

the [demo](./demo.c) above uses `spec-fault` to **detect memory breakpoints and protect critical memory from debuggers**.

## usage

1. compile `lib/spec-fault.c` and include `lib/spec-fault.h`.
2. use `spec_fault_read` to check for read faults.
3. use `spec_fault_write` to check for write faults.
4. a return value of `true` indicates a (possible) fault.

# pitfalls

although the GIF above does look pretty cool:

* like any other timing-based technique, this method is never 100% accurate.
* while a memory breakpoint does cause a fault, not every fault is due to a breakpoint.
* currently, altering a page's flags makes `spec_fault_write` report a fault indefinitely.

please bear these issues in mind when evaluating this project.

## credits
* [@can1357][can1357]'s [`haruspex`][haruspex] - inspiration and the spec-exec trigger

[spectre-meltdown]: https://en.wikipedia.org/wiki/Transient_execution_CPU_vulnerability

[can1357]: https://blog.can.ac
[haruspex]: https://github.com/can1357/haruspex
