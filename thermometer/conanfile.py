from conans import ConanFile, CMake


class MyTempSensorConan(ConanFile):
    name = "thermometer"
    version = "1.0"
    license = "MIT"
    url = ""
    description = "Simple example of MQTT application sending a fake temperature"
    settings = "os", "arch"
    generators = "cmake"
    exports_sources = "src/*"
    requires = ("paho-c/1.2.0@conan/stable",
                "libcurl/7.60.0@bincrafters/stable",
                "json-c/0.13.1@bincrafters/stable")

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder="src")
        cmake.build()

    def package(self):
        self.copy("thermometer", dst="bin", src="bin")

    def deploy(self):
        self.copy("thermometer", src="bin", dst="bin")
