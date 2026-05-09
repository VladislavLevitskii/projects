using System.Reflection.Metadata;
using CommandLineParser.Exceptions;

namespace CommandLineParser
{
    /// <summary>
    /// Provides a builder for configuring and creating option definitions with parameters, aliases, and help
    /// information.
    /// </summary>
    public sealed class OptionDefinitionBuilder
    {
        /// <summary>
        /// Initializes a new instance of the OptionDefinitionBuilder class with the specified option name.
        /// </summary>
        /// <param name="name">The name of the option.</param>
        public OptionDefinitionBuilder(string name) 
        { 
            if (!name.StartsWith('-'))
            {
                _aliases = [name, "-" + name];
            } else
            {
                _aliases = [name];
            }
            _helpProvider = new HelpProvider();
        }
        
        /// <summary>
        /// Associates a help provider with the option definition.
        /// </summary>
        /// <param name="helpProvider">The help provider to associate.</param>
        /// <returns>The current OptionDefinitionBuilder instance.</returns>
        public OptionDefinitionBuilder WithHelp(HelpProvider helpProvider) 
        { 
            _helpProvider = helpProvider; 
            return this; 
        }

        /// <summary>
        /// Adds one or more alternative names for the option. If it doesn't start with '-' it will be implicitly added.
        /// </summary>
        /// <param name="aliases">An array of strings representing the aliases to associate with the option.</param>
        /// <returns>The current OptionDefinitionBuilder instance for method chaining.</returns>
        public OptionDefinitionBuilder WithAlias(params string[] aliases) 
        { 
            var formattedAliases = aliases.Select(a => a.StartsWith('-') ? a : "-" + a);
            var newAliases = formattedAliases.Except(_aliases);

            _aliases.AddRange(newAliases);
            return this; 
        }

        /// <summary>
        /// Adds a parameter definition to the option builder.
        /// </summary>
        /// <typeparam name="Type">The type of the parameter value.</typeparam>
        /// <param name="parameter">The parameter definition to add.</param>
        /// <returns>The updated option definition builder.</returns>
        public OptionDefinitionBuilder AddParameter<Type>(ParameterDefinition<Type> parameter) where Type : notnull
        { 
            _parameterDefinition.Add(parameter); 
            return this;
        }

        /// <summary>
        /// Adds a parameter definition with the specified name and configuration.
        /// </summary>
        /// <typeparam name="Type">The type of the parameter value.</typeparam>
        /// <param name="name">The name of the parameter.</param>
        /// <param name="configure">A delegate to configure the parameter definition.</param>
        /// <returns>The current OptionDefinitionBuilder instance.</returns>
        public OptionDefinitionBuilder AddParameter<Type>(string name, Func<ParameterDefinitionBuilder<Type>, ParameterDefinition<Type>> configure) where Type : notnull
        {
            ParameterDefinitionBuilder<Type> builder = new ParameterDefinitionBuilder<Type>(name);
            ParameterDefinition<Type> parameter = configure(builder);
            _parameterDefinition.Add(parameter);
            return this;
        }

        /// <summary>
        /// Creates and returns an OptionDefinition instance.
        /// </summary>
        /// <returns>An OptionDefinition representing the configured option.</returns>
        public OptionDefinition Build() 
        {  
            return new OptionDefinition(
                _aliases[0],
                _helpProvider,
                _aliases,
                _parameterDefinition);
        }

        /// <summary>
        /// Creates and returns an OptionDefinition instance and assigns it to the out parameter.
        /// </summary>
        /// <param name="def">When this method returns, contains the newly created option definition.</param>
        /// <returns>The same option definition instance that was created.</returns>
        public OptionDefinition Build(out OptionDefinition def)
        {
            def = Build();
            return def;
        }

        private HelpProvider _helpProvider;
        private readonly List<string> _aliases;
        private List<IParameterDefinition> _parameterDefinition = [];
    }

    /// <summary>
    /// Represents a command-line option definition, including its name, help information, and aliases.
    /// </summary>
    public sealed class OptionDefinition
    {
        /// <summary>
        /// Internal constructor for OptionDefinition, used by the OptionDefinitionBuilder to create instances.
        /// </summary>
        internal OptionDefinition(
            string name,
            HelpProvider helpProvider,
            List<string> aliases,
            List<IParameterDefinition> parameterDefinition) 
        { 
            _name = name;
            _helpProvider = helpProvider;
            _aliases = new List<string>(aliases);
            _parameterDefinition = new List<IParameterDefinition>(parameterDefinition);
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
        /// Retrieves the option name.
        /// </summary>
        /// <returns>The option name.</returns>
        internal string GetName()
        {
            return _name;
        }

        private readonly List<string> _aliases;

        /// This is for visibility, CommandParserBuilder should see every alias of the option
        internal IReadOnlyList<string> Aliases => _aliases.AsReadOnly();
        internal IReadOnlyList<IParameterDefinition> ParameterDefinitions => _parameterDefinition.AsReadOnly();

        internal string _name;
        private readonly HelpProvider _helpProvider;
        private readonly List<IParameterDefinition> _parameterDefinition = [];

    }
}