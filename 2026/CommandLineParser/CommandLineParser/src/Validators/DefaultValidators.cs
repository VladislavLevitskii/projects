using CommandLineParser;

namespace DefaultValidators
{
    /// <summary>
    /// Provides validation using one or more user-defined lambda functions.
    /// </summary>
    public sealed class LambdaValidator<T> : IValidator<T>
    {
        /// <summary>
        /// Initializes a new instance of the LambdaValidator class with the specified validation function.
        /// </summary>
        /// <param name="validationFunc">A delegate that defines the validation logic for the value.</param>
        public LambdaValidator(Func<T, bool> validationFunc) 
        { 
            _validationFunc.Add(validationFunc);
        }

        /// <summary>
        /// Initializes a new instance of the LambdaValidator class with the specified validation functions.
        /// </summary>
        /// <param name="validationFunc">A delegate that defines the validation logic for the value.</param>
        /// <param name="additionalValidationFuncs">Additional validation functions to apply to the value.</param>
        public LambdaValidator(Func<T, bool> validationFunc, params Func<T, bool>[] additionalValidationFuncs) 
        { 
            _validationFunc.Add(validationFunc);
            _validationFunc.AddRange(additionalValidationFuncs);
        }

        /// <summary>
        /// Determines whether the specified value meets validation criteria.
        /// </summary>
        /// <param name="value">The value to validate.</param>
        /// <returns><see langword="true"/> if the value is valid; otherwise, <see langword="false"/>.</returns>
        public bool Validate(T value) 
        { 
            return _validationFunc.All(func => func(value));
        }
        
        private readonly List<Func<T, bool>> _validationFunc = new List<Func<T, bool>>();
    }

    /// <summary>
    /// Provides validation for integer values within optional lower and upper bounds.
    /// </summary>
    public sealed class IntValidator : IValidator<int>
    {
        /// <summary>
        /// Initializes a new instance of the IntValidator class with optional lower and upper bounds.
        /// </summary>
        /// <param name="lowerBound">The inclusive lower bound for validation, or null for no lower bound.</param>
        /// <param name="upperBound">The inclusive upper bound for validation, or null for no upper bound.</param>
        public IntValidator(int? lowerBound = null, int? upperBound = null) 
        { 
            _lowerBound = lowerBound;
            _upperBound = upperBound;
        }

        /// <summary>
        /// Determines whether the specified value meets validation criteria.
        /// </summary>
        /// <param name="value">The integer to validate.</param>
        /// <returns>true if the integer is valid; otherwise, false.</returns>
        public bool Validate(int value) 
        {  
            if (_lowerBound.HasValue && value < _lowerBound.Value) return false;
            if (_upperBound.HasValue && value > _upperBound.Value) return false;
            return true;
        }

        private readonly int? _lowerBound;
        private readonly int? _upperBound;
    }

    /// <summary>
    /// Provides validation for string values based on length and regular expression constraints.
    /// </summary>
    public sealed class StringValidator : IValidator<string>
    {
        /// <summary>
        /// Initializes a new instance of the StringValidator class with optional length and pattern constraints.
        /// </summary>
        /// <param name="minLength">The minimum allowed length of the string, or null for no minimum.</param>
        /// <param name="maxLength">The maximum allowed length of the string, or null for no maximum.</param>
        /// <param name="regexPattern">A regular expression pattern the string must match, or null for no pattern.</param>
        public StringValidator(int? minLength = null, int? maxLength = null, string? regexPattern = null) 
        {
            _minLength = minLength;
            _maxLength = maxLength;
            _regexPattern = regexPattern;
        }
        /// <summary>
        /// Determines whether the specified string meets validation criteria.
        /// </summary>
        /// <param name="value">The string to validate.</param>
        /// <returns>true if the string is valid; otherwise, false.</returns>
        public bool Validate(string value) 
        {
            if (value == null || string.IsNullOrEmpty(value)) return false;
            if (_minLength != null && value.Length < _minLength.Value) return false;
            if (_maxLength != null && value.Length > _maxLength.Value) return false;
            if (_regexPattern != null && !System.Text.RegularExpressions.Regex.IsMatch(value, _regexPattern)) return false;
            return true;
        }

        private readonly int? _minLength;
        private readonly int? _maxLength;
        private readonly string? _regexPattern;
    }

    /// <summary>
    /// Provides validation for boolean values.
    /// </summary>
    public sealed class BoolValidator : IValidator<bool>
    {
        /// <summary>
        /// Initializes a new instance of the BoolValidator class with an optional expected value.
        /// </summary>
        /// <param name="expectedValue">The expected boolean value, or null to accept either value.</param>
        public BoolValidator(bool? expectedValue = null)
        {
            _expectedValue = expectedValue;
        }

        /// <summary>
        /// Determines whether the specified boolean value meets validation criteria.
        /// </summary>
        /// <param name="value">The boolean value to validate.</param>
        /// <returns><see langword="true"/> if the value is valid; otherwise, <see langword="false"/>.</returns>
        public bool Validate(bool value) 
        {
            return !_expectedValue.HasValue || value == _expectedValue.Value;
        }

        private readonly bool? _expectedValue;
    }
    
    /// <summary>
    /// Provides validation for string representations of enum values of type T.
    /// </summary>
    /// <typeparam name="T">The enum type to validate.</typeparam>
    public sealed class EnumValidator<T> : IValidator<T> where T : Enum
    {
        /// <summary>
        /// Determines whether the specified value is valid according to the current validation rules.
        /// </summary>
        /// <param name="value">The value to validate.</param>
        /// <returns>true if the value is valid; otherwise, false.</returns>
        public bool Validate(T value) 
        {
            return Enum.IsDefined(typeof(T), value);
        }
    }
}