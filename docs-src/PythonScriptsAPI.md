Python Scripts API                        {#PythonScriptsAPI}
==================

FIXME  This is outdated

%MGE Engine provide two python modules:
 * [MGE](../python/MGE.html) with %MGE python interface (via boost::python).
 * [MGEOgre](../python/MGEOgre.html) with convertion interface between python interface provided by [Ogre via SWIG](../python/_Ogre.html) and %MGE boost::python interface.

%MGE Engine provide global @c ScriptsStoredData python dictionary for script data, that should be stored in save file.
This struct is serialize to json and store in \<ScriptsStoredData\> subnode of \<ScriptsSystem\>.

Both modules are imported into game script context by @ref MGE::ScriptsSystem, so you don't need write `import %MGE` in scripts.
