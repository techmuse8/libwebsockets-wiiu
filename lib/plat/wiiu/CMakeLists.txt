include_directories(.)

list(APPEND SOURCES
    plat/wiiu/plat-wiiu.c
)

# Export back to parent
exports_to_parent_scope()
