
The code in this directory might look a bit strange.
I've taken what was all the code in mute.cpp (the
application) and broke it into a shared library with
all the code.  I changed main to mute_main.  Now
the application has only one line of code in main
that executes mute_main.

The reason for this weird change is that the original
mute.cpp caused a runtime type system problem.
In the explanation of the problem below, I assume that
mute in the original version with all the code in
mute.cpp.

1) mute causes a "RootHandlers" service to come into existance
by name. This service is loaded from the plugin manager and
it does its work.

2) the simple event processing we do (emptysource + tracer service)
function properly

3) mute attempts to get a handle to the "RootHandlers" service
to call one of its member functions.  The problem crashes with an
assertion.

The assertion indicates that the plugin (RootHandlers) was found,
but when the plugin manager attempted to convert the abstract form
in its map to the concrete RootHandlers type, the dynamic_cast
failed and returned null.  This should be impossible if the
program was put together properly, so assert is the right thing
to do here.  We know the dynamic_cast should have succeeded because
we can see the inheritance relationship in the source code and looked
at the types being converted.

What does this mean? It means that there is two definitions of
RootHandlers in the application.  One comes from the
Services/Utilities libraries where the source is and one comes from
main.  Although the types are the same, we end up with two different
"typeinfo" objects in the typeid table.

Why is this happening?  We do not yet know.  We can see duplicated
vtable/typeinfo objects, existing in the symbol table of the executable
and in the symbol table of the FWCoreUtilities library (where
RootHandlers lives).  I checked cmsRun in their latest release and
it has the same organization, but it works!  The only difference
is that we are using the gcc 4.x suite and they are using 3.x.  We
are also using slf5.x and they are still using 4.x.
Maybe the 3.x is more lax in its rules for constructing the symbol
tables and the dynamic linker ends up doing the right thing accidently?

I did notice that we got more missing external dependencies when
compiling the mute shared library.  I corrected these to get the
executable build.  So it appears that library dependencies work
fine, but using objects in shared libraries and also using them
as plugins has problems.

We do not have a good explanation of the root cause of this
problem.
