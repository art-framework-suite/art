use strict;

# Correct arguments to module entry points.
s&,.*?EventSetup.*?\)&\)&;
s&\([^,\)]*EventSetup[^,\)]*\)&\(\)&;
