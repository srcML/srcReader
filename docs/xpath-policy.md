1. Always namespace-qualify srcML elements

   * Use canonical prefixes in emitted XPath.
   * At minimum:

     * `src:` for `http://www.srcML.org/srcML/src`
     * `diff:` for `http://www.srcML.org/srcDiff`
     * `mv:` for `http://www.srcML.org/srcMove`
     * preserve real prefix for other namespaces when available, such as `cpp:`

2. Preserve diff wrappers in the path

   * Do not “skip through” `diff:insert`, `diff:delete`, or other diff-layer elements.
   * If a node is inside a diff wrapper, that wrapper must appear in the XPath.

3. Use `[@filename="..."]` for nested archive file units

   * Root archive container remains `/src:unit`
   * Child file units should be identified as:

     * `/src:unit/src:unit[@filename="main.cpp"]`
   * Do not use positional indexing for nested file units when `@filename` is available.

4. Prefer semantic predicates for stable named program elements

   * Use `[src:name="..."]` when the node has a stable srcML name child.
   * Primary targets:

     * `src:function`
     * `src:function_decl`
     * `src:constructor`
     * `src:destructor`
     * `src:decl`
     * `src:param`
     * `src:typedef`
   * Example:

     * `/src:unit/src:function[src:name="main"]`
     * `/src:unit/src:function[src:name="main"]/src:block/src:block_content/src:decl_stmt[1]/src:decl[src:name="second"]`

5. Fall back to positional `[n]` only when there is no stable semantic anchor

   * If same-named siblings exist and no semantic predicate is used, emit a 1-based positional predicate.
   * Example:

     * `src:decl_stmt[3]`
   * Do not add `[n]` to nodes already uniquely identified by a semantic predicate.

6. Prefer stability over minimal length

   * The goal is a path that is readable and reproducible for tests.
   * A slightly longer XPath is preferred if it avoids ambiguity.

7. Only use semantic predicates when the extracted name is simple and stable

   * If the `src:name` subtree is empty, mixed, or likely unstable due to diff markup or unusual syntax, do not force `[src:name="..."]`.
   * Fall back to positional indexing instead.

