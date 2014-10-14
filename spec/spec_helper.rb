ROOT = File.expand_path(File.dirname(__FILE__))
$LOAD_PATH.unshift( File.expand_path("../lib", ROOT) )

require "epeg"

TEST_JPEG = File.join(ROOT, "fixtures", "einstein.jpg")
