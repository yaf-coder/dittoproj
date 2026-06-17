/** @type {import('tailwindcss').Config} */
export default {
  content: ['./index.html', './src/**/*.{js,jsx}'],
  theme: {
    extend: {
      colors: {
        apple: {
          blue:      '#2D4A6A',
          'blue-dk': '#1E3550',
          gray:      '#ECEEF4',
          'gray-2':  '#DDE0EA',
          text:      '#2A2A2E',
          sub:       '#8D8FA8',
          red:       '#C0392B',
          green:     '#2E7D52',
          leather:   '#9A6040',
          dust:      '#E8D5C8',
          emperor:   '#4B4B4B',
        },
      },
      fontFamily: {
        sans: [
          '-apple-system', 'BlinkMacSystemFont', '"SF Pro Display"',
          '"SF Pro Text"', '"Helvetica Neue"', 'Arial', 'sans-serif',
        ],
        cursive: ['"Playfair Display"', 'serif'],
      },
      borderRadius: {
        '4xl': '2rem',
        '5xl': '2.5rem',
      },
      boxShadow: {
        'apple-sm': '0 2px 12px rgba(0,0,0,0.08)',
        'apple':    '0 4px 30px rgba(0,0,0,0.10)',
        'apple-lg': '0 8px 60px rgba(0,0,0,0.14)',
      },
      letterSpacing: {
        tight2: '-0.03em',
      },
    },
  },
  plugins: [],
}
