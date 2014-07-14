set(boost_program_options_SOURCES
	libs/program_options/src/positional_options.cpp
	libs/program_options/src/utf8_codecvt_facet.cpp
	libs/program_options/src/parsers.cpp
	libs/program_options/src/convert.cpp
	libs/program_options/src/options_description.cpp
	libs/program_options/src/cmdline.cpp
	libs/program_options/src/variables_map.cpp
	libs/program_options/src/value_semantic.cpp
)
add_library(boost_program_options
	${boost_program_options_SOURCES}
)
set_target_properties(boost_program_options PROPERTIES COMPILE_FLAGS -fPIC)

set(boost_serialization_SOURCES
    libs/serialization/src/extended_type_info.cpp
    libs/serialization/src/basic_pointer_iserializer.cpp
    libs/serialization/src/utf8_codecvt_facet.cpp
    libs/serialization/src/codecvt_null.cpp
    libs/serialization/src/extended_type_info_typeid.cpp
    libs/serialization/src/basic_iarchive.cpp
    libs/serialization/src/basic_pointer_oserializer.cpp
    libs/serialization/src/basic_iserializer.cpp
    libs/serialization/src/shared_ptr_helper.cpp
    libs/serialization/src/basic_archive.cpp
    libs/serialization/src/archive_exception.cpp
    libs/serialization/src/void_cast.cpp
    libs/serialization/src/basic_serializer_map.cpp
    libs/serialization/src/basic_oserializer.cpp
    libs/serialization/src/basic_oarchive.cpp
    libs/serialization/src/basic_text_oprimitive.cpp
    libs/serialization/src/basic_text_iprimitive.cpp
    libs/serialization/src/basic_xml_archive.cpp
    libs/serialization/src/xml_archive_exception.cpp
    libs/serialization/src/xml_grammar.cpp
    libs/serialization/src/xml_oarchive.cpp
    libs/serialization/src/xml_iarchive.cpp
)
add_library(boost_serialization
    ${boost_serialization_SOURCES}
)
set_target_properties(boost_serialization PROPERTIES COMPILE_FLAGS -fPIC)

set(boost_system_SOURCES
    libs/system/src/error_code.cpp
)
add_library(boost_system
    ${boost_system_SOURCES}
)
set_target_properties(boost_system PROPERTIES COMPILE_FLAGS -fPIC)

set(boost_iostreams_SOURCES
	libs/iostreams/src/zlib.cpp
)
add_library(boost_iostreams
    ${boost_iostreams_SOURCES}
)
set_target_properties(boost_iostreams PROPERTIES COMPILE_FLAGS -fPIC)
