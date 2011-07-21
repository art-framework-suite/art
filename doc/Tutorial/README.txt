
For the image addition in index.md,  I could have used the
markdown
 ![alt text](../images/nova_art.001.png "art")
but they give no controls for sizing to window width and
I am not knowledgable enough with markdown to use the
stylesheet stuff to help (if possible).

I also ran into the problem of skipping the image directory
and just causing it to copy the files from it.

-----------------------

Talk for the g-2 meeting.  Most of this stuff is in the tutorial
already and I will add no more material now.

0) key terms
1) configuring a program
2) running the program
3) writing a new module
4) accessing the event
5) making histograms
6) looking at the data in the file

what about this format?
1) analysis module / filter module code and the skeleton generator
2) build involving making a library with just that in it
3) data file that is produced and its structure including data products
4) configuring the program to use modules and writing files

In addition, I need to discuss the organization of the ROOT file.
Go to ~novasoft on oink if they want to see an example of the
library files that are generated.
  (releases/development/lib/Linux2.6-GCC)

If we get to it, essential elements of the workflow language
must be discussed.  In particular, that named paths are defined
containing sequences of modules.  Each of those paths can
be used to generate a true or false, indicating if the processing
in that path found the thing it was designed to find.
Output modules and analyzers live in a special "end path".  Output
modules are confiured to accept events for output from paths
and the default is to access all events from every path.

I also understand this to be an introductory talk to discuss
what the framework can do.

Questions to lead out with:
1) what tasks are you going to be doing?

