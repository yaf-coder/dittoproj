import { useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { motion, AnimatePresence } from 'framer-motion'
import { api } from '../api/client'
import { useAuth } from '../store/AuthContext'
import Button from '../components/Button'
import Input  from '../components/Input'

const STEPS = [
  {
    key:        'age',
    title:      'How old are you?',
    subtitle:   'You must be 18 or older.',
    inputProps: { placeholder: 'Age', type: 'number', min: 18, max: 120 },
  },
  {
    key:        'location',
    title:      'Where are you based?',
    subtitle:   'City or region, e.g. "Austin, TX".',
    inputProps: { placeholder: 'Location', type: 'text' },
  },
]

const slide = {
  enter:  { opacity: 0, x: 60 },
  center: { opacity: 1, x: 0 },
  exit:   { opacity: 0, x: -60 },
}

export default function Onboarding() {
  const { user, setUser } = useAuth()
  const navigate = useNavigate()

  const [step, setStep]     = useState(0)
  const [values, setValues] = useState({ age: '', location: '' })
  const [error, setError]   = useState('')
  const [saving, setSaving] = useState(false)

  const current = STEPS[step]
  const value   = values[current.key]
  const isLast  = step === STEPS.length - 1

  function handleChange(e) {
    setError('')
    setValues(v => ({ ...v, [current.key]: e.target.value }))
  }

  async function handleNext() {
    if (!value.toString().trim()) { setError('This field is required.'); return }
    if (current.key === 'age' && (Number(value) < 18 || Number(value) > 120)) {
      setError('Age must be between 18 and 120.')
      return
    }

    if (!isLast) { setStep(s => s + 1); return }

    setSaving(true)
    try {
      await api.updateProfile({
        name:         user?.name ?? '',
        age:          Number(values.age),
        location:     values.location.trim(),
        phone_number: user?.phone_number ?? '',
      })
      const me = await api.getMe()
      setUser(me)
      navigate('/questions', { replace: true })
    } catch (e) {
      setError(e.message)
    } finally {
      setSaving(false)
    }
  }

  function handleKeyDown(e) {
    if (e.key === 'Enter') handleNext()
  }

  const progress = (step + 1) / STEPS.length

  return (
    <div className="h-dvh flex flex-col px-6 safe-top">
      {/* Progress bar */}
      <div className="pt-6 pb-8">
        <div className="h-1 bg-apple-gray-2 rounded-full overflow-hidden">
          <motion.div
            className="h-full bg-apple-blue rounded-full"
            animate={{ width: `${progress * 100}%` }}
            transition={{ type: 'spring', stiffness: 260, damping: 30 }}
          />
        </div>
      </div>

      {/* Step content */}
      <div className="flex-1 flex flex-col">
        <AnimatePresence mode="wait">
          <motion.div
            key={step}
            variants={slide}
            initial="enter"
            animate="center"
            exit="exit"
            transition={{ duration: 0.28, ease: [0.25, 0.46, 0.45, 0.94] }}
            className="flex-1 flex flex-col"
          >
            <h1 className="text-[32px] font-bold tracking-tight2 text-apple-text mb-2">
              {current.title}
            </h1>
            <p className="text-[17px] text-apple-sub mb-8">{current.subtitle}</p>

            <Input
              value={value}
              onChange={handleChange}
              onKeyDown={handleKeyDown}
              autoFocus
              {...current.inputProps}
            />

            {error && (
              <motion.p
                initial={{ opacity: 0, y: -8 }}
                animate={{ opacity: 1, y: 0 }}
                className="text-apple-red text-[13px] mt-3 px-1"
              >
                {error}
              </motion.p>
            )}
          </motion.div>
        </AnimatePresence>
      </div>

      {/* Footer */}
      <div className="pb-10 flex flex-col gap-3">
        <Button fullWidth onClick={handleNext} disabled={saving}>
          {saving ? 'Saving…' : isLast ? 'Get started' : 'Continue'}
        </Button>
        {step > 0 && (
          <Button fullWidth variant="ghost" onClick={() => setStep(s => s - 1)}>
            Back
          </Button>
        )}
      </div>
    </div>
  )
}
