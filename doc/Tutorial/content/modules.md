---
title: Modules
prev: /dataproducts
next: /sources
---

Modules
=======

* These are the user-written processing elements of the system. Each has its own configuration.  

* Each module processes one *Event* at a time.

* Four types of modules:
  1. **Producers** - request data products, insert new data products
  2. **Filters** - request data products and can alter further processing using return values
  3. **Analyzers** - request data products, do not create new ones; make histograms, etc.
  4. **Output** - store data products to files (provided by framework)