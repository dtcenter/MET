import re

ATCF_ID_REGEX = re.compile(
    r"(?P<basin>[a-zA-Z]{2})(?P<storm_number>\d{2})(?P<year>\d{4})")
ATCF_FILENAME_STEM_REGEX = re.compile(
    r"(?P<track_type>[a-zA-Z])(?P<basin>[a-zA-Z]{2})(?P<storm_number>\d{2})(?P<year>\d{4})"
)
