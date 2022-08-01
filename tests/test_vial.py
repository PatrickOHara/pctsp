"""Tests for vials"""

from pctsp.vial import (
    AlgorithmName,
    EXACT_ALGORITHMS,
    HEURISTIC_ALGORITHMS,
    RELAXATION_ALGORITHMS,
)


def test_algorithm_name_lists():
    """Test every algorithm in included in the list of algorithms"""
    all_algorithms = EXACT_ALGORITHMS + HEURISTIC_ALGORITHMS + RELAXATION_ALGORITHMS
    for algorithm in AlgorithmName:
        assert algorithm in all_algorithms
