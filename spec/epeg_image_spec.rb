require "spec_helper"
require "tempfile"

describe Epeg::Image do
  context "when loading an invalid jpeg file" do
    it "should raise an error" do
      invalid_jpeg = Tempfile.new(%w{invalid .jpg})
      expect{ Epeg::Image.open(invalid_jpeg.path)      }.to raise_error
      expect{ Epeg::Image.from_blob(invalid_jpeg.read) }.to raise_error
    end
  end

  context "when loading a valid jpeg file" do
    around(:each) do |example|
      Epeg::Image.quality = 75
      @image = Epeg::Image.open(TEST_JPEG)
      @output_image = Tempfile.new(%w{out .jpg})

      example.run

      @output_image.unlink
    end

    it "should have well-defined width and height" do
      expect(@image.width ).to eq(20)
      expect(@image.height).to eq(10)
    end

    it "should create new image from blob" do
      data = File.open(TEST_JPEG, "rb"){ |f| f.read }
      expect{ Epeg::Image.from_blob(data) }.not_to raise_error
    end

    it "should return blob" do
      blob = @image.to_blob

      File.open(@output_image.path, "wb"){ |f| f.write(blob) }
      @image_from_blob = Epeg::Image.open(@output_image.path)

      expect(@image_from_blob.width ).to eq(20)
      expect(@image_from_blob.height).to eq(10)
    end

    it "should resize an image" do
      @image.resize(2, 2)
      @image.write(@output_image.path)

      @resized_image = Epeg::Image.open(@output_image.path)

      expect(@resized_image.width ).to eq(2)
      expect(@resized_image.height).to eq(2)
    end

    it "should resize an image to fit specific dimensions" do
      @image.resize_to_fit(8,4)
      @image.write(@output_image.path)

      @resized_image = Epeg::Image.open(@output_image.path)

      expect(@resized_image.width ).to eq(8)
      expect(@resized_image.height).to eq(4)
    end

    it "should resize an image to fill specific dimensions" do
      @image.resize_to_fill(8,8)
      @image.write(@output_image.path)

      @resized_image = Epeg::Image.open(@output_image.path)

      expect(@resized_image.width ).to eq(16)
      expect(@resized_image.height).to eq(8)
    end

    it "should crop an image" do
      @image.crop(8, 8, 0, 0)
      @image.write(@output_image.path)

      @another_image = Epeg::Image.open(TEST_JPEG)
      @another_image.crop(8, 8)
      @another_output_image = Tempfile.new(%w{out2 .jpg})
      @another_image.write(@another_output_image.path)

      @cropped_image = Epeg::Image.open(@output_image.path)
      @another_cropped_image = Epeg::Image.open(@another_output_image.path)

      expect(@cropped_image.width ).to eq(8)
      expect(@cropped_image.height).to eq(8)

      expect(@another_cropped_image.width ).to eq(8)
      expect(@another_cropped_image.height).to eq(8)

      expect(@cropped_image.to_blob).not_to eq(@another_cropped_image.to_blob)
    end

    it "should not write image after closing" do
      @image.close

      expect(@image).to be_closed
      expect{@image.write(@output_image.path)}.to raise_error
      expect{@image.to_blob}.to raise_error
    end

    it "should use default quality" do
      expect(@image.quality).to eq(75)
    end

    it "should set valid quality" do
      @image.quality = 100
      @image.write(@output_image.path)
      high_qu_file_size = File.size(@output_image)

      @image = Epeg::Image.open(TEST_JPEG)
      @image.quality = 1
      @image.write(@output_image.path)
      low_qu_file_size  = File.size(@output_image)

      expect(@image.quality).to eq(1)
      expect(high_qu_file_size).to be > low_qu_file_size
    end

    it "should not set invalid quality" do
      expect{ @image.quality = -1 }.to raise_error
      expect{ @image.quality = 101 }.to raise_error
    end
  end
end
