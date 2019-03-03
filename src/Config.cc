namespace np {
      struct LogConfig {
          struct Levels {
              int message = 0;
              int param = 0;
          };

          std::vector<std::pair<std::string, Levels>> levels_by_name;
          std::vector<std::pair<unsigned, Levels>> levels_by_depth;
          Levels default_levels;
      };

      std::atomic unsigned config_timestamp;
      std::shared_ptr<LogConfig> config_ptr;

      // we could pimpl the public api. Or we could virtualize it.
      struct Config {
        setLevels(unsigned char messages, unsigned char parameters);
        setLevelForName(std::string_view name, unsigned char messages, unsigned char parameters);
        setLevelForDepth(unsigned int depth, unsigned char messages, unsigned char parameters);

        void apply() const;

      private:
        std::unique_ptr<LogConfig> cfg;
      };

      // ok, so I am logger N, I am nested at depth D, and my most recent configuration is from version V
      bool isCurrent(V);
      std::pair<Levels, unsigned> getLevels(N, D);
}
