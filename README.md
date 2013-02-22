pr0nget
=======


About
-----

pr0nget is a exercise and an unpolished PoC I did for teh lulz

It finds pr0n on imgur, displays it and allows the user to save images


How it works
------------

pr0nget guesses imgur images by bruteforcing their URLs

Once a valid URL is found, the corresponding thumbnail is downloaded

Some statistical data are comupted over the pic which determine whether it's pr0n or not

The actual classifier is derived with a genetic approach (see extra/GA-classifier.pl) run over about 1000 manually classified samples (20% pr0n)

Due to the lazy arse approach and the limited metrics used, the number of false positives is rather high (i.e. you get some non-pr0n too).


How to compile
--------------

In order to build pr0nget you'll need a working Qt4 build environment (tested on Qt4.8)

The steps are:
<pre>
qmake
make
</pre>

Under windows, if you have Qt available for static linkage, you can build a static executable with:

<pre>
qmake "CONFIG+=allstatic"
make
</pre>


How to use
----------

Run it :)

The space bar loads more images from the queue (i.e. if moar images are available).

Double click on a thumbnail to get the big picture (as well as the ability to save it).

Middle click on a thumbnail to open the corresponding imgur page.

Right click on a thumbnail to revert it to the previous image (if any).

Use backspace to revert all the pics to their previous image.


