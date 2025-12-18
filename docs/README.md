# Trusted-CPP Documentation

This directory contains the documentation for the Trusted-CPP project, built with Jekyll and hosted on GitHub Pages.

## Structure

The documentation is organized as follows:
- Root directory (`/`) - Contains the English documentation (default language)
- `ru/` - Contains the Russian translations
- `_layouts/` - Page layouts
- `assets/` - CSS stylesheets and other assets
- `_config.yml` - Jekyll configuration file

## Local Development

To run the documentation site locally, you'll need to have Ruby and Bundler installed. Then follow these steps:

1. Navigate to the `docs` directory:
   ```
   cd docs
   ```

2. Install the required gems:
   ```
   bundle install
   ```

3. Start the Jekyll server:
   ```
   bundle exec jekyll serve
   ```

4. Open your browser and navigate to `http://localhost:4000` to view the site.

## Multilingual Support

The documentation is available in both English (default) and Russian:
- English: Accessible at the root path `/`
- Russian: Accessible under the `/ru/` path

Use the language selector in the navigation bar to switch between languages.

For more information about the Trusted-CPP project, visit our [GitHub repository](https://github.com/afteri-ru/trusted-cpp).
