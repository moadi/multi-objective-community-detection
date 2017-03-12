ROOT=$(pwd)

##################################################
# Build the entire tree
##################################################
function m() {
  scons --debug-build --lldb $@
}

function mr() {
	scons --lldb $@
}
