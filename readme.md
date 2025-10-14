# JSON Library

A lightweight C library for parsing, creating, and printing JSON data.

This library demonstrates how a **lexer** and **parser** can work together to
process structured text. It also provides an educational example showing the
utility of parser generators for building real-world tools that process JSON.

---

## Overview

The library provides a small, self-contained API for working with JSON values.
It supports all standard JSON types—objects, arrays, strings, numbers,
booleans, and null—and exposes a simple interface for parsing, constructing,
and printing JSON data.

- **Header:** `json.h`  
  Declares all public API functions for value creation, access, and memory
  management.

- **Implementation:** `json.c`, `scanner.l`, `grammar.y`  
  Implements parsing, printing, and internal data structures using a parser
  generator.

