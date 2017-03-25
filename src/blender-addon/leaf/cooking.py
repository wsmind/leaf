import bpy
import sys
import tempfile
import os.path
import hashlib
import subprocess
import json

cooker = None

class Cooker():
    def __init__(self):
        self.cache_root =  os.path.join(tempfile.gettempdir(), "LeafCache")
        
        # create the cache root folder if necessary
        if not os.path.exists(self.cache_root):
            os.mkdir(self.cache_root)
        
        print("Cooker cache: " + self.cache_root)
        self.processors = {}

    def register_processor(self, type, processor):
        processor_hash = self._hashfile(processor.exe_path)
        self.processors[type] = (processor, processor_hash)

    def cook(self, type, path, options):
        print("Cooking: " + path)

        try:
            processor, processor_hash = self.processors[type]

            source_path = bpy.path.abspath(path)
            source_hash = self._hashfile(source_path)
            
            options_hash = self._hashdict(options)

            cache_key = processor_hash + "_" + source_hash + "_" + options_hash
            cache_path = os.path.join(self.cache_root, cache_key)

            # look in the cache first
            if os.path.exists(cache_path):
                with open(cache_path, "rb") as f:
                    return f.read()
                    
            # otherwise, run the processor
            target_path = os.path.join(bpy.app.tempdir, next(tempfile._get_candidate_names()))
            processor.process(source_path, target_path, options)

            # cache result
            os.rename(target_path, cache_path)

            with open(cache_path, "rb") as f:
                return f.read()

        except:
            print("Failed to cook " + path + ":")
            print(str(sys.exc_info()))
            return b''

    def _hashfile(self, path):
        md5 = hashlib.md5()
        with open(path, "rb") as f:
            md5.update(f.read())
        return md5.hexdigest()
    
    def _hashdict(self, d):
        md5 = hashlib.md5()
        md5.update(json.dumps(d, sort_keys=True).encode("utf-8"))
        return md5.hexdigest()

class ImageProcessor():
    def __init__(self):
        script_dir = os.path.dirname(__file__)
        self.exe_path = os.path.join(script_dir, "LeafTextureCompressor.exe")

    def process(self, source_path, target_path, options):
        args = [
            self.exe_path,
            "-repeat",
            "-dds10"
        ]

        if not options["cuda"]:
            args.append("-nocuda")

        if options["linear"]:
            args.append("-linear")
        else:
            args.append("-color")
            args.append("-srgb")
    
        if options["normal_map"]:
            args.append("-normal")
            args.append("-bc1n")
        else:
            args.append("-bc1")

        args.append(source_path)
        args.append(target_path)

        print("running: " + str(args))
        subprocess.run(args)
