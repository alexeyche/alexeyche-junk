#!/usr/bin/env bash
latex ./review && bibtex ./review && latex ./review && latex ./review
