
if [ "$DGL_VERSION" != "" ]; then
    version="-DDGL_VERSION=$DGL_VERSION"
else
    version=""
fi

mkdir -p ../build && cd ../build && CXX=g++-4.7 CC=gcc-4.7 cmake ../src $version && make package
