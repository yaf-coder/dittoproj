import { useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { motion } from 'framer-motion'
import { api } from '../api/client'
import { useAuth } from '../store/AuthContext'
import Button from '../components/Button'
import Input  from '../components/Input'

const EDUCATION_OPTIONS = [
  { label: 'High School',      value: 'high_school' },
  { label: 'Some College',     value: 'some_college' },
  { label: "Bachelor's",       value: 'bachelors' },
  { label: "Master's",         value: 'masters' },
  { label: 'PhD / Doctorate',  value: 'phd' },
]

const GENDER_OPTIONS = [
  { label: 'Man',              value: 'male' },
  { label: 'Woman',            value: 'female' },
  { label: 'Non-binary',       value: 'non_binary' },
  { label: 'Other',            value: 'other' },
  { label: 'Prefer not to say', value: 'prefer_not_to_say' },
]

const POLITICAL_OPTIONS = [
  { label: 'Very Liberal',     value: 0.0 },
  { label: 'Liberal',          value: 0.25 },
  { label: 'Moderate',         value: 0.5 },
  { label: 'Conservative',     value: 0.75 },
  { label: 'Very Conservative', value: 1.0 },
]

function SelectPill({ options, selected, onSelect }) {
  return (
    <div className="flex flex-wrap gap-2">
      {options.map(opt => {
        const isSelected = selected === opt.value
        return (
          <button
            key={opt.value}
            type="button"
            onClick={() => onSelect(isSelected ? null : opt.value)}
            className={[
              'px-4 py-2 rounded-full text-[15px] font-medium transition-colors duration-150',
              isSelected
                ? 'bg-apple-blue text-white'
                : 'bg-apple-gray text-apple-text hover:bg-apple-gray-2',
            ].join(' ')}
          >
            {opt.label}
          </button>
        )
      })}
    </div>
  )
}

function Question({ number, total, title, subtitle, children }) {
  return (
    <motion.div
      initial={{ opacity: 0, y: 16 }}
      animate={{ opacity: 1, y: 0 }}
      transition={{ duration: 0.4, delay: number * 0.07 }}
      className="flex flex-col gap-3"
    >
      <div>
        <p className="text-[12px] font-semibold text-apple-blue uppercase tracking-wide mb-1">
          Question {number} of {total}
        </p>
        <h2 className="text-[20px] font-bold tracking-tight2 text-apple-text">{title}</h2>
        {subtitle && <p className="text-[14px] text-apple-sub mt-0.5">{subtitle}</p>}
      </div>
      {children}
    </motion.div>
  )
}

export default function Questions() {
  const { user, setUser } = useAuth()
  const navigate = useNavigate()

  const [form, setForm] = useState({
    education_level:  user?.education_level  ?? null,
    study_location:   user?.study_location   ?? '',
    age:              user?.age              ?? '',
    gender:           user?.gender           ?? null,
    political_belief: user?.political_belief ?? null,
  })
  const [saving, setSaving] = useState(false)
  const [error, setError]   = useState('')

  function set(key, val) {
    setForm(f => ({ ...f, [key]: val }))
  }

  async function handleSave() {
    setSaving(true)
    setError('')
    try {
      const payload = {}
      if (form.education_level !== null)  payload.education_level  = form.education_level
      if (form.study_location.trim())     payload.study_location   = form.study_location.trim()
      if (form.age !== '' && form.age !== null) {
        const n = Number(form.age)
        if (n < 18 || n > 120) { setError('Age must be between 18 and 120.'); setSaving(false); return }
        payload.age = n
      }
      if (form.gender !== null)           payload.gender           = form.gender
      if (form.political_belief !== null) payload.political_belief = form.political_belief

      if (Object.keys(payload).length > 0) {
        await api.enrichProfile(payload)
        const me = await api.getMe()
        setUser(me)
      }
      navigate('/discover', { replace: true })
    } catch (e) {
      setError(e.message)
    } finally {
      setSaving(false)
    }
  }

  return (
    <div className="flex-1 flex flex-col overflow-y-auto">
      {/* Header */}
      <div className="px-6 pt-14 pb-2">
        <motion.h1
          initial={{ opacity: 0, y: 16 }}
          animate={{ opacity: 1, y: 0 }}
          className="text-[28px] font-bold tracking-tight2 text-apple-text"
        >
          Tell us about yourself
        </motion.h1>
        <motion.p
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          transition={{ delay: 0.1 }}
          className="text-[15px] text-apple-sub mt-1"
        >
          All questions are optional.
        </motion.p>
      </div>

      {/* Questions */}
      <div className="px-6 py-6 flex flex-col gap-8">
        <Question number={1} total={5} title="What is your education level?">
          <SelectPill
            options={EDUCATION_OPTIONS}
            selected={form.education_level}
            onSelect={v => set('education_level', v)}
          />
        </Question>

        <Question number={2} total={5} title="Where did you study?" subtitle="School, university, or program name.">
          <Input
            placeholder="e.g. University of Texas"
            type="text"
            value={form.study_location}
            onChange={e => set('study_location', e.target.value)}
          />
        </Question>

        <Question number={3} total={5} title="What is your age?">
          <Input
            placeholder="Age"
            type="number"
            min={18}
            max={120}
            value={form.age}
            onChange={e => set('age', e.target.value)}
          />
        </Question>

        <Question number={4} total={5} title="What is your gender identity?">
          <SelectPill
            options={GENDER_OPTIONS}
            selected={form.gender}
            onSelect={v => set('gender', v)}
          />
        </Question>

        <Question number={5} total={5} title="What is your political identity?">
          <SelectPill
            options={POLITICAL_OPTIONS}
            selected={form.political_belief}
            onSelect={v => set('political_belief', v)}
          />
        </Question>

        {error && (
          <motion.p
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            className="text-apple-red text-[13px]"
          >
            {error}
          </motion.p>
        )}
      </div>

      {/* Footer */}
      <div className="px-6 pb-10 pt-2 flex flex-col gap-3">
        <Button fullWidth onClick={handleSave} disabled={saving}>
          {saving ? 'Saving…' : 'Save & continue'}
        </Button>
        <Button fullWidth variant="ghost" onClick={() => navigate('/discover', { replace: true })}>
          Skip for now
        </Button>
      </div>
    </div>
  )
}
