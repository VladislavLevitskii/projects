using CommandLineParser.Exceptions;

namespace CommandLineParser
{

    /// Used for grouping every parameter (possibly with different type) in one collection
    internal interface IParameterDefinition
    {
        HelpProvider GetHelp();
        string GetName();
        object GetParameter(string? value);
        bool IsRequired { get; }
    }

    /// <summary>
    /// Provides a builder for configuring and creating a parameter definition with validation, parsing, and help
    /// information.
    /// </summary>
    /// <typeparam name="T">The type of the parameter value.</typeparam>
    public sealed class ParameterDefinitionBuilder<T> where T : notnull
    {
        /// <summary>
        /// Initializes a new instance of the ParameterDefinitionBuilder class with the specified parameter name.
        /// </summary>
        /// <param name="name">The name of the parameter to define.</param>
        public ParameterDefinitionBuilder(string name) 
        { 
            _name = name;
            _helpProvider = new HelpProvider();
        }

        /// <summary>
        /// Marks the parameter as required or optional.
        /// If this method is not called, the parameter is optional by default.
        /// </summary>
        /// <param name="isRequired">true to mark the parameter as required; otherwise, false.</param>
        /// <returns>The current builder instance.</returns>
        public ParameterDefinitionBuilder<T> IsRequired(bool isRequired = true) 
        { 
            _isRequired = isRequired;
            return this;
        }

        /// <summary>
        /// Associates a help provider with the parameter definition builder.
        /// </summary>
        /// <param name="helpProvider">The help provider to associate.</param>
        /// <returns>The updated parameter definition builder.</returns>
        public ParameterDefinitionBuilder<T> WithHelp(HelpProvider helpProvider) 
        { 
            _helpProvider = helpProvider;
            return this;
        }

        /// <summary>
        /// Adds a validator to the parameter definition.
        /// </summary>
        /// <param name="validator">The validator to apply to the parameter.</param>
        /// <returns>The current builder instance with the validator applied.</returns>
        public ParameterDefinitionBuilder<T> WithValidator(IValidator<T> validator) 
        { 
            _validator = validator;
            return this;
        }

        /// <summary>
        /// Sets the parser used to convert input values to the specified type.
        /// </summary>
        /// <param name="parser">The parser to use for converting input values.</param>
        /// <returns>A builder configured with the specified parser.</returns>
        public ParameterDefinitionBuilder<T> WithParser(IParser<T> parser) 
        { 
            _parser = parser;
            return this;
        }

        /// <summary>
        /// Sets default value for the parameter, which will be used if the parameter is not provided in the command line arguments.
        /// </summary>
        /// <param name="defaultValue"></param>
        /// <returns>A builder configured with the default value</returns>
        public ParameterDefinitionBuilder<T> WithDefault(T defaultValue) 
        { 
            _defaultValue = defaultValue;
            _hasDefaultValue = true;
            return this;
        }

        /// <summary>
        /// Creates and returns a new parameter definition instance.
        /// </summary>
        /// <returns>A parameter definition of type T.</returns>
        public ParameterDefinition<T> Build() 
        { 
            IParser<T>? finalParser = _parser ?? GetDefaultParser();

            // Parser is required, validator is optional
            if (finalParser == null)
            {
                throw new CommandLineParserException("no default parser for that parameter, define your own");
            }
            
            return new ParameterDefinition<T>(
            _name, 
            _isRequired, 
            _helpProvider, 
            _validator, 
            finalParser,
            _defaultValue,
            _hasDefaultValue);
        }

        /// <summary>
        /// Creates and returns a new parameter definition instance and assigns it to the out parameter.
        /// </summary>
        /// <param name="def">When this method returns, contains the newly created parameter definition.</param>
        /// <returns>The same parameter definition instance that was created.</returns>
        public ParameterDefinition<T> Build(out ParameterDefinition<T> def) 
        { 
            def = Build();
            return def; 
        }

        /// <summary>
        /// Gets the default parser for type T.
        /// </summary>
        /// <returns>The default parser, or null if no default exists.</returns>
        private static IParser<T>? GetDefaultParser()
        {
            Type t = typeof(T);

            if (t == typeof(int))
            {
                return (IParser<T>)(object)new DefaultParsers.IntParser();
            }
            else if (t == typeof(string))
            {
                return (IParser<T>)(object)new DefaultParsers.StringParser();
            }
            else if (t == typeof(bool))
            {
                return (IParser<T>)(object)new DefaultParsers.BoolParser();
            }
            else if (t.IsEnum)
            {
                Type enumParserType = typeof(DefaultParsers.EnumParser<>).MakeGenericType(t);
                return (IParser<T>)Activator.CreateInstance(enumParserType)!;
            } else
            {
                return null;
            }
        }

        private readonly string _name;
        private HelpProvider _helpProvider;
        private IValidator<T>? _validator;
        private IParser<T>? _parser;
        private T? _defaultValue;
        private bool _isRequired = false;
        private bool _hasDefaultValue = false;

    }
    /// <summary>
    /// Represents a definition for a parameter, including its name, requirement status, help information, validation,
    /// and parsing logic.
    /// </summary>
    /// <typeparam name="T">The type of the parameter value.</typeparam>
    public sealed class ParameterDefinition<T> : IParameterDefinition where T : notnull
    {
        /// <summary>
        /// Internal constructor for ParameterDefinition, used by the ParameterDefinitionBuilder to create instances.
        /// </summary>
        internal ParameterDefinition(
            string name,
            bool isRequired,
            HelpProvider helpProvider,
            IValidator<T>? validator,
            IParser<T>? parser,
            T? defaultValue,
            bool hasDefaultValue)
        { 
            _name = name;
            _isRequired = isRequired;
            _helpProvider = helpProvider;
            _validator = validator;
            _parser = parser;
            _defaultValue = defaultValue;
            _hasDefaultValue = hasDefaultValue;
        }

        /// <summary>
        /// Retrieves a parameter of the specified type from the provided value and arguments.
        /// </summary>
        /// <param name="value">The string representation of the parameter.</param>
        /// <returns>The parameter converted to the specified type.</returns>
        internal T GetParameter(string? value) 
        {  
            if (string.IsNullOrEmpty(value))
            {
                if (_hasDefaultValue)
                {
                    return _defaultValue!;
                }

                if (_isRequired)
                {
                    throw new CommandLineParserParsingException($"Missing required parameter: '{_name}'");
                }

                return default!;
            }
            
            try
            {
                T parsedResult = _parser!.Parse(value);

                if (_validator != null && !_validator.Validate(parsedResult))
                {
                    throw new CommandLineParserValidationException($"Invalid value for parameter '{_name}': '{value}'");
                }

                return parsedResult;
            }
            catch (CommandLineParserParsingException)
            {
                throw;
            }
            catch
            {
                throw new CommandLineParserParsingException($"Invalid value for parameter '{_name}': '{value}'");
            }

        }

        /// <summary>
        /// Retrieves the associated help provider.
        /// </summary>
        /// <returns>A HelpProvider instance containing help information.</returns>
        public HelpProvider GetHelp() 
        { 
            return _helpProvider;
        }

        /// <summary>
        /// Retrieves the parsed value for the supplied input.
        /// </summary>
        /// <param name="value">The raw string value to parse.</param>
        /// <returns>The parsed value.</returns>
        object IParameterDefinition.GetParameter(string? value)
        {
            return GetParameter(value)!;
        }

        /// <summary>
        /// Retrieves the parameter name.
        /// </summary>
        /// <returns>The parameter name.</returns>
        string IParameterDefinition.GetName()
        {
            return _name;
        }

        private readonly bool _isRequired;
        private readonly string _name;
        private readonly HelpProvider _helpProvider;
        private readonly IValidator<T>? _validator;
        private readonly IParser<T>? _parser;
        private readonly T? _defaultValue;
        private readonly bool _hasDefaultValue;

        internal string Name => _name;
        bool IParameterDefinition.IsRequired => _isRequired;    
    }
}