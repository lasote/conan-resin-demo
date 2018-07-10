FROM resin/armv7hf-python

WORKDIR /usr/src/app
ENV LD_LIBRARY_PATH /usr/src/app/bin

RUN sudo pip install conan --upgrade
RUN conan remote add iot https://api.bintray.com/conan/lasote/testing-iot -i 0 \
   && conan profile new default --detect \
   && conan profile update settings.arch=armv7hf default \
   && conan profile update settings.arch_build=armv7hf default \
   && conan install mosquitto/1.4.15@bincrafters/stable --update

RUN sed -i "s|#user mosquitto|user root|g" bin/mosquitto.conf \
   && sed -i "s|#allow_anonymous true|allow_anonymous false|g" bin/mosquitto.conf \
   && sed -i "s|#password_file|password_file /data/mosquitto/mosquitto_pass|g" bin/mosquitto.conf

COPY run.sh /usr/src/app/

CMD ["/usr/src/app/run.sh"]