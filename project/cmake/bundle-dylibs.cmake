file(GLOB DYLIBS "${BIN_DIR}/*.dylib")
foreach(dylib ${DYLIBS})
    file(COPY ${dylib} DESTINATION ${TARGET_DIR})
endforeach()
