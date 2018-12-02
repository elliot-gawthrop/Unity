Example 4
=========

This example project gives an example of testing functions which rely on the outcome of other
lower level functions. If the lower level functions are broken, it is not possible to correctly
test any of the higher level functions - their outcomes will be marked as inconclusive. This
allows us to quickly determine the root cause of the issue, by looking at the tests which have
conclusively failed.

You can build and test using rake. The rake version will let you test with gcc or a couple
versions of IAR.  You can tweak the yaml files to get those versions running.

Ruby is required if you're using the rake version (obviously).  This version shows off most of
Unity's advanced features (automatically creating test runners, fancy summaries, etc.)
Without ruby, you have to maintain your own test runners.  Do that for a while and you'll learn
why you really want to start using the Ruby tools.
