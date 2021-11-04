set(DOXYGEN_PROJECT_LOGO  logo.png)
set(LAYOUT_FILE DoxygenLayout.xml)
doxygen_add_docs(
    docs
    ${PROJECT_SOURCE_DIR}/docs
)