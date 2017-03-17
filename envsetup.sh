ROOT=$(pwd)

##################################################
# Build the entire tree
##################################################
function m() {
  pushd $ROOT > /dev/null
  scons --debug-build --lldb $@
  popd > /dev/null
}

function mr() {
	pushd $ROOT > /dev/null
	scons --lldb $@
	popd > /dev/null
}
