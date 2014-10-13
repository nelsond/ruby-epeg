# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'epeg/version'

Gem::Specification.new do |spec|
  spec.name          = "ruby-epeg"
  spec.version       = Epeg::VERSION
  spec.authors       = ["Nelson Darkwah Oppong"]
  spec.email         = ["ndo@felixnelson.de"]
  spec.summary       = %q{Ruby extension for the epeg library which provides facilities for scaling JPEG images very quickly.}
  spec.homepage      = "http://github.com/nelsond/ruby-epeg"
  spec.license       = "MIT"

  spec.files         = `git ls-files -z`.split("\x0")
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]
  spec.extensions    = ["ext/epeg/extconf.rb"]

  spec.add_dependency "rake-compiler", "~> 0.9"

  spec.add_development_dependency "bundler", "~> 1.7"
  spec.add_development_dependency "rake", "~> 10.0"
  spec.add_development_dependency "rspec", "~> 3.0"
end
