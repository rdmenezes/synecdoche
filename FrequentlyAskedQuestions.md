

# What does Synecdoche mean? #

A [synecdoche](http://en.wiktionary.org/wiki/synecdoche) is a figure of speech where:
  * A part stands for a whole
  * An individual stands for a class
  * A material stands for a thing
(Or the reverse.)

In distributed computing, this idea is everywhere. We talk about a task, meaning a fragment run on a single computer, or a whole distributed grid working on a unified task. **Synecdoche** embodies this idea.

# How is Synecdoche pronounced? #

**syn·ec·do·che** (sĭ-nĕk'də-kē)

Listen to an [audio clip](http://en.wiktionary.org/wiki/Image:en-us-synecdoche.ogg).

# Why fork from BOINC? #

We didn't take this decision lightly. We have spent considerable time and effort in trying to work with BOINC and improve it. However, some major problems made forking the only viable option:

  * Many patches ignored or misapplied
  * Premature releases with known bugs
  * Minor feature requests ignored
  * Berkeley lock-in
  * An ever-growing bug list
  * Poor usability
  * Poor coding standards

The Linux Information Project has this to say about [project forks](http://www.bellevuelinux.org/project_fork.html):
> _"...the possibility of forking is actually one of the greatest strengths of the free software development model. One reason is that widespread awareness of the possibility of forking and the consequent problems that can result can serve as a strong incentive to achieve consensus and work together cooperatively. Another is that such awareness provides an additional incentive to developers to do outstanding work. In addition, when forking occurs, the possibility exists that it will eventually result in a productive cross pollination, or even a merger, of the branches."_

Our hope is that the BOINC project will benefit from our efforts, but if not, Synecdoche can benefit from past and future BOINC developments, while making our own improvements at the same time.

# Will you support GPUs/CUDA? #

Currently there are no plans to add any kind of GPU support, but we don't rule out the possibility of doing it in the future. We will not use BOINC's current code for GPU scheduling, though.

# BOINC rejected my idea. Will you accept it? #

## I have a feature request ##

We will reconsider any idea, whether BOINC rejected it or not. However, there is no guarantee a feature request will be accepted. Some features are better suited as add-ons, or are simply antithetical to the way Synecdoche is designed. Even if we don't accept a feature, we are happy to discuss it and search for a solution to the underlying problem.

## I have a patch ##

We will try out and study any patches submitted to Synecdoche, and provide feedback whether we apply the patch as provided or not. The caveats for feature requests apply to patches, too.

We welcome patches for existing Synecdoche and BOINC issues, as well as new features. Talk to us first if you want to improve the usefulness of your patch, and avoid duplicated effort.

# Can I set up a Synecdoche project? #

Not yet. Our initial releases will focus on the client parts of Synecdoche. However, we plan to support the server features as soon as practically possible. If you want to help with this, or set up a Synecdoche project, please contact us.

# I want to help. What should I do? #

Please contact us. Join the [developer list](http://groups.google.com/group/synecdoche-dev) or talk to us at [irc://irc.freenode.net/synecdoche](irc://irc.freenode.net/synecdoche)

Synecdoche is looking for talented developers, technical writers, designers and translators. We have large and small tasks for all skill levels, so please tell us which areas of Synecdoche interest you.