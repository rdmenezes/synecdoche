# Code Documentation #

Synecdoche uses [Doxygen](http://www.doxygen.org/) for code documentation. Documentation is generated automatically from special comments in the source code.

## Comment Style ##

Doxygen supports many different documentation styles, but for consistency, we only use one style. Synecdoche uses the C++ comment style, with three slashes denoting a documentation block. Commands are indicated with a back slash. The first line of the description, up to the first period, is used as a brief description. For clarity, start a new line after the first period.

```
/// This is a brief summary.
/// This is a detailed description of the class or class member. It can
/// be as long as you need.
///
/// \param[out] name1 Description of inbound argument.
/// \param[in,out] name2 Description of in/out argument.
/// \return Description of return value.
```

This is a minimal example. You can use any of the [Doxygen commands](http://www.stack.nl/~dimitri/doxygen/commands.html) to provide as much information as you need.

For documenting enum members or private fields, you may want to put the comment after the member, instead of before.
```
int var; ///< Detailed description after the member.
```

## Guidelines ##

  * State the obvious. It may not be obvious to other readers.
  * Avoid writing [undocumentation](http://www.codinghorror.com/blog/archives/000451.html).
  * Keep documentation up to date. Inaccurate documentation is worse than no documentation at all.