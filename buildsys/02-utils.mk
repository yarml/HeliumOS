line-count:
	@$(PY) $(PYDIR)line_count.py

line-police:
	@$(PY) $(PYDIR)line_police.py

.PHONY: line-count
