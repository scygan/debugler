set(boost_program_options_SOURCES
    libs/program_options/src/positional_options.cpp
    libs/program_options/src/utf8_codecvt_facet.cpp
    libs/program_options/src/parsers.cpp
    libs/program_options/src/convert.cpp
    libs/program_options/src/options_description.cpp
    libs/program_options/src/winmain.cpp
    libs/program_options/src/split.cpp
    libs/program_options/src/cmdline.cpp
    libs/program_options/src/variables_map.cpp
    libs/program_options/src/config_file.cpp
    libs/program_options/src/value_semantic.cpp
)
add_library(boost_program_options
    ${boost_program_options_SOURCES}
)
set_target_properties(boost_program_options PROPERTIES COMPILE_FLAGS -fPIC)

set(boost_regexp_SOURCES
    libs/regex/src/usinstances.cpp
    libs/regex/src/wide_posix_api.cpp
    libs/regex/src/static_mutex.cpp
    libs/regex/src/cregex.cpp
    libs/regex/src/wc_regex_traits.cpp
    libs/regex/src/w32_regex_traits.cpp
    libs/regex/src/cpp_regex_traits.cpp
    libs/regex/src/fileiter.cpp
    libs/regex/src/winstances.cpp
    libs/regex/src/c_regex_traits.cpp
    libs/regex/src/regex_debug.cpp
    libs/regex/src/posix_api.cpp
    libs/regex/src/regex_traits_defaults.cpp
    libs/regex/src/icu.cpp
    libs/regex/src/instances.cpp
    libs/regex/src/regex.cpp
    libs/regex/src/regex_raw_buffer.cpp
)
add_library(boost_regexp
    ${boost_regexp_SOURCES}
)
set_target_properties(boost_regexp PROPERTIES COMPILE_FLAGS -fPIC)

set(boost_serialization_SOURCES
    libs/serialization/src/extended_type_info.cpp
    libs/serialization/src/xml_grammar.cpp
    libs/serialization/src/basic_pointer_iserializer.cpp
    libs/serialization/src/utf8_codecvt_facet.cpp
    libs/serialization/src/polymorphic_oarchive.cpp
    libs/serialization/src/codecvt_null.cpp
    libs/serialization/src/binary_oarchive.cpp
    libs/serialization/src/basic_xml_archive.cpp
    libs/serialization/src/extended_type_info_typeid.cpp
    libs/serialization/src/xml_woarchive.cpp
    libs/serialization/src/basic_iarchive.cpp
    libs/serialization/src/stl_port.cpp
    libs/serialization/src/basic_pointer_oserializer.cpp
    libs/serialization/src/xml_archive_exception.cpp
    libs/serialization/src/xml_wgrammar.cpp
    libs/serialization/src/extended_type_info_no_rtti.cpp
    libs/serialization/src/xml_wiarchive.cpp
    libs/serialization/src/basic_iserializer.cpp
    libs/serialization/src/shared_ptr_helper.cpp
    libs/serialization/src/basic_archive.cpp
    libs/serialization/src/basic_text_woprimitive.cpp
    libs/serialization/src/text_iarchive.cpp
    libs/serialization/src/archive_exception.cpp
    libs/serialization/src/text_wiarchive.cpp
    libs/serialization/src/void_cast.cpp
    libs/serialization/src/polymorphic_iarchive.cpp
    libs/serialization/src/text_oarchive.cpp
    libs/serialization/src/text_woarchive.cpp
    libs/serialization/src/basic_text_iprimitive.cpp
    libs/serialization/src/binary_iarchive.cpp
    libs/serialization/src/basic_serializer_map.cpp
    libs/serialization/src/binary_woarchive.cpp
    libs/serialization/src/xml_oarchive.cpp
    libs/serialization/src/basic_text_wiprimitive.cpp
    libs/serialization/src/binary_wiarchive.cpp
    libs/serialization/src/xml_iarchive.cpp
    libs/serialization/src/basic_text_oprimitive.cpp
    libs/serialization/src/basic_oserializer.cpp
    libs/serialization/src/basic_oarchive.cpp
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

set(boost_thread_SOURCES
    libs/thread/src/pthread/thread.cpp
    libs/thread/src/pthread/once.cpp
)
add_library(boost_thread
    ${boost_thread_SOURCES}
)
set_target_properties(boost_thread PROPERTIES COMPILE_FLAGS -fPIC)


