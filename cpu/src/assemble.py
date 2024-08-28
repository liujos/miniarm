from armasm.assemble import AssemblyParser
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('filename')
args = parser.parse_args()

ap = AssemblyParser()
ap.parse_file(args.filename)

