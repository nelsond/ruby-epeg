ROOT = File.expand_path(File.dirname(__FILE__))

require "bundler/gem_tasks"

require "rspec/core/rake_task"
RSpec::Core::RakeTask.new(:spec => [:compile])
task :default => :spec
task :test    => :spec

require "rake/extensiontask"
Rake::ExtensionTask.new("epeg")

task :valgrind => [:compile] do
  system("valgrind --leak-check=yes ruby #{ROOT}/spec/runner.rb")
end
