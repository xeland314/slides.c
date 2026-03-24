import unittest
import os
import sys

def run_tests():
    # Asegurarse de que el directorio actual esté en el path para importar los bindings
    sys.path.append(os.path.abspath(os.path.dirname(__file__)))
    
    loader = unittest.TestLoader()
    suite = loader.discover(start_dir=os.path.dirname(__file__), pattern='test_*.py')
    
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    if not result.wasSuccessful():
        sys.exit(1)

if __name__ == "__main__":
    run_tests()
