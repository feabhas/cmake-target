#!/bin/bash

# The Build all script is intended for checking projects and solutions
# uploaded to a GIT repo for continuous integration checks.

DIR=$(dirname "$0")
exec "$DIR"/build-one.sh --all

