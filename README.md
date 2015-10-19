/nds/null
=========

Presenting a single threaded, though pretty fast, super web scale (not really, just forcing in buzz words) HTTP 1.0 server.

The bare minimum is there to comply with HTTP/1.0 (I believe), so it actually works well enough to be used with a standard web browser (though not really useful).

Building
--------

First of all, copy the include/payload.h.template file to include/payload.h

Then if you're building for the Nintendo DS, just run make (assuming you have DevKitPro in your path, make will complain about it anyway in case it's not there).

For Linux or Windows (Cygwin), you have to gcc the C files together (sorry, no Makefile, I'm bad at those).
