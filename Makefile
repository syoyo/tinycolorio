all:
	#clang++ -std=c++11 -Weverything -Wno-c++98-compat -Wno-padded test_tcio.cc
	clang++ -std=c++11 -Weverything -Wno-c++98-compat -Wno-padded test_ocio_config.cc yaml/Yaml.cpp
