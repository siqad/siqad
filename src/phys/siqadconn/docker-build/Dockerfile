# Resources:
# Good consilidated guide on required components: https://thomastrapp.com/blog/building-a-pypi-package-for-a-modern-cpp-project/
# Project that uses pypa/manylinux and CIs for cross platform compilation: moderngl/moderngl

#FROM quay.io/pypa/manylinux1_x86_64:latest as build_x86_64
FROM dockcross/manylinux1-x64:latest as build_x86_64
#RUN yum -y install swig boost-devel    # provided versions are too old

# no need to compile cmake from source as dockcross/manylinux1 provides a fairly new version

# download and compile GCC 8.3.0
#RUN curl -L0 'https://mirror.csclub.uwaterloo.ca/gnu/gcc/gcc-8.3.0/gcc-8.3.0.tar.gz' > gcc-8.3.0.tar.gz
#RUN tar -xzf gcc-8.3.0.tar.gz
#RUN cd gcc-8.3.0 && ./contrib/download_prerequisites && ./configure --enable-languages=c,c++ --disable-multilib && make -j24 && make install
#ENV PATH="/usr/local/bin:$PATH"
#ENV CC=/usr/local/bin/gcc
#ENV CXX=/usr/local/bin/g++

# download and compile Boost 1.72.0
RUN yum -y install libicu libicu-devel
RUN curl -L0 'https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz' > boost_1_72_0.tar.gz
RUN tar -xzf boost_1_72_0.tar.gz
RUN cd boost_1_72_0 && ./bootstrap.sh && ./b2 -j24 --with-system cxxflags="-fPIC" runtime-link=static variant=release link=static install

# download and compile SWIG 3.0.12
RUN yum -y install pcre pcre-devel
RUN curl -L0 'https://sourceforge.net/projects/swig/files/swig/swig-3.0.12/swig-3.0.12.tar.gz/download' > swig-3.0.12.tar.gz
RUN tar -xzf swig-3.0.12.tar.gz
RUN cd swig-3.0.12 && ./configure --disable-perl --disable-ruby --disable-csharp --disable-r --disable-java && make -j24 && make install

# unpack siqadtools and compile a wheel for each available Python version
COPY siqadtools-*.tar.gz siqadtools.tar.gz
RUN tar --strip-components=1 -zxvf siqadtools.tar.gz
RUN /opt/python/cp35-cp35m/bin/python setup.py bdist_wheel && auditwheel repair dist/*.whl
RUN /opt/python/cp36-cp36m/bin/python setup.py bdist_wheel && auditwheel repair dist/*.whl
RUN /opt/python/cp37-cp37m/bin/python setup.py bdist_wheel && auditwheel repair dist/*.whl

#RUN /opt/python/cp38-cp38m/bin/python setup.py bdist_wheel
#RUN /opt/python/cp35-cp35m/bin/python setup.py bdist_wheel && auditwheel repair dist/*.whl && rm dist/*.whl
#RUN /opt/python/cp36-cp36m/bin/python setup.py bdist_wheel && auditwheel repair dist/*.whl && rm dist/*.whl
#RUN /opt/python/cp37-cp37m/bin/python setup.py bdist_wheel && auditwheel repair dist/*.whl && rm dist/*.whl
#RUN /opt/python/cp38-cp38m/bin/python setup.py bdist_wheel && auditwheel repair dist/*.whl && rm dist/*.whl
