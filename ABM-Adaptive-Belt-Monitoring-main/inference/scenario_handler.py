import logging
import pickle
import numpy as np
from pathlib import Path

logger = logging.getLogger(__name__)

class ScenarioHandler:
    """
    Handles predefined scenarios without needing sensor data.
    Maps scenario names directly to inference results.
    """
    
    def __init__(self):
        self.models = self.load_models()
        
        # Define all scenarios based on your requirements
        self.approach1_scenarios = {
            "loose_unstable_good": {
                "tension": "Loose",
                "stability": "Unstable",
                "health": "Good",
                "approach": "Approach 1",
                "description": "Loose tension with unstable operation, but motor health is good"
            },
            "optimal_stable_good": {
                "tension": "Optimal",
                "stability": "Stable",
                "health": "Good",
                "approach": "Approach 1",
                "description": "Optimal conditions with stable operation and good health"
            },
            "optimal_unstable_warning": {
                "tension": "Optimal",
                "stability": "Unstable",
                "health": "Warning",
                "approach": "Approach 1",
                "description": "Optimal tension but unstable, requiring attention"
            },
            "tight_stable_critical": {
                "tension": "Tight",
                "stability": "Stable",
                "health": "Critical",
                "approach": "Approach 1",
                "description": "Tight tension causing critical motor health condition"
            }
        }
        
        self.approach2_scenarios = {
            "adjustment_loose": {
                "tension": "Loose",
                "stability": "N/A",
                "health": "Requires Tightening",
                "approach": "Approach 2",
                "description": "Belt is loose, needs adjustment"
            },
            "adjustment_optimal": {
                "tension": "Optimal",
                "stability": "N/A",
                "health": "No Adjustment Needed",
                "approach": "Approach 2",
                "description": "Belt tension is optimal"
            },
            "adjustment_tight": {
                "tension": "Tight",
                "stability": "N/A",
                "health": "Requires Loosening",
                "approach": "Approach 2",
                "description": "Belt is too tight, needs loosening"
            }
        }
        
        # Combine all scenarios
        self.all_scenarios = {
            **self.approach1_scenarios,
            **self.approach2_scenarios
        }
        
        logger.info(f"✅ Loaded {len(self.all_scenarios)} scenarios")
        self.log_available_scenarios()
    
    def load_models(self):
        """Load all AI models from the models folder"""
        models = {}
        models_path = Path("models")
        
        if not models_path.exists():
            logger.warning(f"⚠️ Models folder not found: {models_path}")
            return models
        
        model_files = [
            "stability_energy_model.pkl",
            "se_computer (1).pkl",
            "feature_extractor(3) (1).pkl",
            "feature_scaler(3) (1).pkl",
            "tension_classifier(3) (1).pkl"
        ]
        
        for model_file in model_files:
            model_path = models_path / model_file
            if model_path.exists():
                try:
                    with open(model_path, 'rb') as f:
                        models[model_file] = pickle.load(f)
                    logger.info(f"✅ Loaded model: {model_file}")
                except Exception as e:
                    logger.error(f"❌ Failed to load {model_file}: {e}")
            else:
                logger.warning(f"⚠️ Model not found: {model_file}")
        
        return models
    
    def log_available_scenarios(self):
        """Log all available scenarios for reference"""
        logger.info("\n📋 Available Scenarios:")
        logger.info("\n--- APPROACH 1 (Stability-Health) ---")
        for key, scenario in self.approach1_scenarios.items():
            logger.info(f"  • {key}: {scenario['description']}")
        
        logger.info("\n--- APPROACH 2 (Adjustment) ---")
        for key, scenario in self.approach2_scenarios.items():
            logger.info(f"  • {key}: {scenario['description']}")
        logger.info("")
    
    def process_scenario(self, scenario_name):
        """
        Process a scenario by name and return the predefined result.
        In a real implementation with models, you could:
        1. Generate synthetic features based on scenario
        2. Run through models
        3. Return predictions
        
        For POC, we return predefined mappings.
        """
        scenario_name = scenario_name.lower().strip()
        
        if scenario_name not in self.all_scenarios:
            logger.error(f"❌ Unknown scenario: {scenario_name}")
            return self.get_error_result(scenario_name)
        
        scenario = self.all_scenarios[scenario_name]
        logger.info(f"✅ Processing scenario: {scenario_name}")
        logger.info(f"   Description: {scenario['description']}")
        
        # For POC: Return predefined results
        # For production: Use models to generate predictions
        result = {
            "scenario": scenario_name,
            "approach": scenario["approach"],
            "tension": scenario["tension"],
            "stability": scenario["stability"],
            "health": scenario["health"],
            "description": scenario["description"]
        }
        
        # Optional: Add model inference if you want to use the loaded models
        if self.models:
            result["model_inference"] = self.run_model_inference(scenario_name)
        
        return result
    
    def run_model_inference(self, scenario_name):
        """
        Optional: Run actual model inference if needed.
        Generate synthetic features based on scenario and predict.
        """
        # This is where you'd use your loaded models
        # For now, return a placeholder
        return {
            "used_models": list(self.models.keys()),
            "note": "Model inference can be added here if needed for POC"
        }
    
    def get_error_result(self, scenario_name):
        """Return error result for unknown scenario"""
        available = list(self.all_scenarios.keys())
        return {
            "scenario": scenario_name,
            "approach": "Error",
            "tension": "N/A",
            "stability": "N/A",
            "health": "Unknown Scenario",
            "description": f"Scenario '{scenario_name}' not found",
            "available_scenarios": available[:5]  # Return first 5 as hint
        }
    
    def get_all_scenarios(self):
        """Return all available scenarios"""
        return list(self.all_scenarios.keys())