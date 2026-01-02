include(ExternalProject)

ExternalProject_Add(
	libgphoto2
	
	GIT_REPOSITORY "https://github.com/VSF1/libgphoto2"
	GIT_TAG "master"

	UPDATE_COMMAND ""
	PATCH_COMMAND ""
	
	CONFIGURE_HANDLED_BY_BUILD true

	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libgphoto2

	CONFIGURE_COMMAND autoreconf -i <SOURCE_DIR> --install --symlink && <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
	BUILD_COMMAND make -j
	#CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:${GLOBAL_OUTPUT_PATH}/libgphoto2
)
