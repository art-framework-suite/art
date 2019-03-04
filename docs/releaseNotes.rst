|release| release notes
=======================

* Dependency on the *hep_concurrency* package has been added for handling thread-safety issues with the SQLite facilities.
* A ``cet::getenv("VarName", std::nothrow)`` function has been added, which returns an empty string if the environment variable does not exist.
* Additional ``cet::search_path`` functions have been added that allow explicit specification regarding whether an environment variable or a path has been provided as an std::string argument to the function.
* A ``cet::exception_category_matcher`` utility has been added that can be used within Catch unit tests.
