---
title: Data Products
prev: /edm
---

Data Products
=============

* Almost all C++ objects can be stored into an *Event* (the objects must not contain bare pointers)

* Most products will be **collections**

* Navigation classes provide the replacement for bare pointers

* There are many ways to "label" the products to allow for later queries by processing elements

* Each data product has a configurable label that can be used in downstream processing queries

* The build system must be told which classes are stored in the *Event*, and must generate "dictionaries"
  which are used to support persistency in Root files.