#! /usr/bin/env python3

import os
import shutil

OUTPUT_DIR = os.environ['MET_TEST_OUTPUT']
TRUTH_DIR = os.environ['MET_TEST_TRUTH']
DIFF_DIR = os.environ['MET_TEST_DIFF']

files_to_copy = set()

with open('/met/logs/comp_dir.log', 'r') as file_handle:
  file_content = file_handle.read()

missing_section, *test_sections = file_content.split('\n# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #\n')

# parse list of missing files
if 'ERROR:' in missing_section:
  for missing_group in missing_section.split('ERROR:')[1:]:
    rel_paths = missing_group.splitlines()[1:]
    if OUTPUT_DIR in missing_group:
      top_dir = TRUTH_DIR
    elif TRUTH_DIR in missing_group:
      top_dir = OUTPUT_DIR
    else:
      continue
    for rel_path in rel_paths:
      files_to_copy.add(os.path.join(top_dir, rel_path.strip()))

# parse file paths out of sections that have errors
error_sections = [item for item in test_sections if 'ERROR:' in item]
for error_section in error_sections:
  for line in error_section.splitlines():
    for item in line.split():
      if OUTPUT_DIR in item or TRUTH_DIR in item:
        files_to_copy.add(item)

print(f"Found {len(files_to_copy)} diff files")

# add extension for output/truth and copy files to diff directory
for filename in files_to_copy:
  output_path, extension = os.path.splitext(filename)
  if OUTPUT_DIR in output_path:
    output_path = f'{output_path}_OUTPUT{extension}'
    output_path = output_path.replace(OUTPUT_DIR, DIFF_DIR)
  elif TRUTH_DIR in output_path:
    output_path = f'{output_path}_TRUTH{extension}'
    output_path = output_path.replace(TRUTH_DIR, DIFF_DIR)
  else:
    continue
  print(f"Copy {filename} to {output_path}")
  output_dir = os.path.dirname(output_path)
  if not os.path.exists(output_dir):
    os.makedirs(output_dir)
  shutil.copyfile(filename, output_path)
