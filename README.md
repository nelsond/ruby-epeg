# ruby-epeg ![Travis CI - build status](https://travis-ci.org/nelsond/ruby-epeg.svg?branch=master)

Ruby extension for the epeg library which provides facilities for scaling JPEG images very quickly.

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'ruby-epeg', :require => 'epeg'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install ruby-epeg

## Example

```ruby
require "epeg"

# Get and set the global default jpeg quality
Epeg::Image.default_quality # => 85
Epeg::Image.default_quality = 90

# Create an image object from the file some-image.jpg
image = Epeg::Image.open("./some-image.jpg")

# You can get the width and height without loading any pixels
"some-image.jpeg is #{image.width}x#{image.height}px"
# => some-image.jpeg is 1000x500

# Resize the image to 50x50, set the quality to 50 and save it
image.resize(50,50).quality(50).write("./some-other-image.jpg")

# The image is now closed and you can't do any further operations
image.closed? # => true

# Load the same image from a buffer
File.open("./some-image.jpg") do |file|
  image = Epeg::Image.from_blob(file.read)
end

# Resize to fill 50x50px (i. e. width and height are at
# least 50px with preserving the aspect ratio) and return the blob
image.resize_to_fill(50,50).to_blob

# Let's crop the resized image
image = Epeg::Image.open("./some-other-image.jpg")
image.crop(50, 50).write("./final-image.jpg")
```
