/** @type {import('tailwindcss').Config} */
export default {
  content: ['./index.html', './src/**/*.{js,jsx}'],
  theme: {
    extend: {
      colors: {
        apple: {
          blue:      '#007AFF',
          'blue-dk': '#0071E3',
          gray:      '#F5F5F7',
          'gray-2':  '#E8E8ED',
          text:      '#1D1D1F',
          sub:       '#6E6E73',
          red:       '#FF3B30',
          green:     '#34C759',
        },
      },
      fontFamily: {
        sans: [
          '-apple-system', 'BlinkMacSystemFont', '"SF Pro Display"',
          '"SF Pro Text"', '"Helvetica Neue"', 'Arial', 'sans-serif',
        ],
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
