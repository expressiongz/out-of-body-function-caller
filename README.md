# remote-process-function-caller
Calls process functions remotely in allocated memory.
This should be specifically compiled using x86-MSVC in release mode, otherwise you may get rel calls instead of a direct call ( e.g FF D1 / call ecx ) and will have to fix the relative address.
# explanation
(thank you to gogo1000 for suggesting this and without me even asking sending me screenshots of some of the code he used to make this!)
The purpose of this is to be able to call a process's functions without getting your module detected. It's a simple and useful feature I'll be implementing into my memory editing library soon.

(note that the way I did it is somewhat lazy, though it still works well)
