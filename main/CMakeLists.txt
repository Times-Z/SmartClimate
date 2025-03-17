# Recherche automatiquement tous les fichiers .c
file(GLOB_RECURSE SOURCES "*.c")

# Inclut automatiquement tous les dossiers qui contiennent des .h
file(GLOB_RECURSE INCLUDE_DIRS_LIST "*.h")
set(INCLUDE_DIRS "")
foreach(header ${INCLUDE_DIRS_LIST})
    get_filename_component(dir ${header} DIRECTORY)
    list(APPEND INCLUDE_DIRS ${dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
)
