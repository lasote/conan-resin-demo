FROM resin/armv7hf-python

WORKDIR /usr/src/app
ENV LD_LIBRARY_PATH /usr/src/app/bin

RUN pip install conan --upgrade
RUN conan remote add iot https://api.bintray.com/conan/lasote/testing-iot -i 0 \
   && conan profile new default --detect \
   && conan profile update settings.arch=armv7hf default \
   && conan profile update settings.arch_build=armv7hf default \
   && conan install thermometer/1.0@conan/stable --update && echo "installed"

COPY run.sh /usr/src/app/

CMD ["/usr/src/app/run.sh"]