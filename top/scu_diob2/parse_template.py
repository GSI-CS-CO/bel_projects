import json
from mako.template import Template
from mako.lookup import TemplateLookup
import argparse
import re

def render_template(template_file: str, output_file: str, json_file: str, configuration: str, directory: str) -> bool:
	"""
	Render a Mako template using configuration from a JSON file.

	:param template_file: Path to the Mako template
	:param json_file: Path to the JSON config
	:param option_name: Optional argument passed to template
	:return: Rendered template as string
	"""
	# Load JSON configuration
	with open(json_file, "r") as f:
		data = json.load(f)

	# Create lookup
	lookup = TemplateLookup(directories=[directory])
	
	# Read and render
	template = lookup.get_template(template_file)

	# Read template
	#with open(template_file, "r") as f:
	#	template_text = f.read()

	# Render template
	#template = Template(template_text, lookup=lookup)

	output_text = template.render(data=data, configuration=configuration)

	# Write output
	with open(directory + "/" + output_file, "w", newline='\n') as f:
		f.write(output_text)
		
	return True


def main():
	parser = argparse.ArgumentParser(description="Render Mako template with JSON config")
	parser.add_argument("template_file", help="Path to the Mako template file")
	parser.add_argument("output_file", help="Path to output file")
	parser.add_argument("json_file", help="Path to JSON configuration file")
	parser.add_argument("--config", help="Optional argument: configuration name", default="default")
	parser.add_argument("--dir", help="Optional argument: root directory for template and output", default=".")
	args = parser.parse_args()

	result = render_template(args.template_file, args.output_file, args.json_file, args.config, args.dir)

	print("Result: " + str(result))


if __name__ == "__main__":
	main()
