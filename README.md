### srcReader

C++ implementation of libxml2's xmlTextReader interface for srcML.

The following is an example API usage to copy a srcML document

```C++
srcml_reader reader("srcml.xml");
srcml_writer writer("copy.xml");

for(const srcml_node & node : reader) {
    writer.write(node);
}
```

### XPath locators

Locators retain readable name and filename conditions followed by a position,
for example `src:function[src:name='process'][2]`. That position counts siblings
matching both the namespace/element name and the condition, not all functions.
Unique names also receive `[1]`. Elements without a usable condition retain
their position among siblings with the same namespace URI and local name.
Archive locators omit the outer archive unit; this convention is unchanged.

From this repository, run `python3 test/run_xpath_tests.py --binary
build/bin/print_xpaths`. Add `--verify-resolution` to independently check that
every emitted locator identifies its own XML element using `lxml`. In the
SrcMLBuildTemplate workspace, build and run these checks inside Docker through
`../bin/srcml-dev-shell`; the workspace's schema-validation Python target
already supplies `lxml`.
