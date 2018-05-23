### srcMLTextReader

C++ implementation of libxml2's xmlTextReader interface for srcML.

The following is an example API usage to copy a srcML document

```C++
srcml_reader reader("srcml.xml");
srcml_writer writer("copy.xml");

for(const srcml_node & node : reader) {
    writer.write(node);
}
```
